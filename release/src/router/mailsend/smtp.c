/*
**  SMTP routines for mailsend - a simple mail sender via SMTP
**
**  Limitations and Comments:
**    SMTP RFC: RFC-821
**    Also look at: TCP/IP Illustraged Vol 1 by Richard Stevens
**
**    Written mainly for NT, Unix has this kind of tools in hundreds.
**
**  Development History:
**      who                  when           why
**      muquit@muquit.com    Mar-26-2001    first cut
*/

#include "mailsend.h"

static SOCKET
    smtp_socket=INVALID_SOCKET;

static char
    buf[BUFSIZ];

static int
    break_out=0;

#ifdef WINNT
static BOOL 
    WINAPI CntrlHandler(DWORD CtrlEvent);
#else
#define closesocket close
#endif /* WINNT */

void smtpDisconnect(void)
{
    if (smtp_socket != INVALID_SOCKET)
        closesocket(smtp_socket);
}
/* connect to SMTP server and returns the socket fd */
static SOCKET smtpConnect(char *smtp_server,int port)
{
    SOCKET
        sfd;

    sfd=clientSocket(smtp_server,port);
    if (sfd == INVALID_SOCKET)
    {
        errorMsg("Could not connect to SMTP server \"%s\" at port %d\n",
                smtp_server,port);
        return (INVALID_SOCKET);
    }

    /* save it. we'll need it to clean up */
    smtp_socket=sfd;

    return (sfd);
}

/* read SMTP response. returns 0 on success, -1 on failure */
static int smtpResponse(int sfd)
{
    int
        n;

    char
        buf[BUFSIZ];

    memset(buf,0,sizeof(buf));
    n=sockGets(sfd,buf,sizeof(buf)-1);

    showVerbose("%s\n",buf);

    if (*buf == '1' || *buf == '2' || (*buf == '3' && *(buf+3) == A_SPACE))
    {
        return (0);
    }

    return (-1);
}



/* SMTP: HELO */
static int smtpHelo(int sfd,char *helo_domain)
{
    /* read off the greeting */
    smtpResponse(sfd);


    memset(buf,0,sizeof(buf));
    (void) snprintf(buf,sizeof(buf)-1,"HELO %s\r\n",helo_domain);

    showVerbose(">>> %s",buf);

    sockPuts(sfd,buf);
    return (smtpResponse(sfd));
}

/* SMTP: MAIL FROM */
static int smtpMailFrom(int sfd,char *from)
{
    memset(buf,0,sizeof(buf));
    (void) snprintf(buf,sizeof(buf)-1,"MAIL FROM: <%s>\r\n",from);

    showVerbose(">>> %s",buf);

    sockPuts(sfd,buf);
    return (smtpResponse(sfd));
}

/* SMTP: quit */
static int smtpQuit(int sfd)
{
    sockPuts(sfd,"QUIT\r\n");

    showVerbose("QUIT\r\n");

    return (smtpResponse(sfd));
}

/* SMTP: RSET */
/* aborts current mail transaction and cause both ends to reset */
static int smtpRset(int sfd)
{
    sockPuts(sfd,"RSET\r\n");
    return (smtpResponse(sfd));
}



/* SMTP: RCPT TO */
static int smtpRcptTo(int sfd)
{
    Sll
        *l,
        *al;

    Address
        *a;

    char
        *x;

    al=getAddressList();
    
    for (l=al; l; l=l->next)
    {
        a=(Address *) l->data;
        if (! a)
            return(-1);
        if (! a->address)
            return(-1);

        memset(buf,0,sizeof(buf));
        x=getenv("NOTIFY_RCPT");
        if (x != NULL)
        {
            /* MS Exchange has it */
            showVerbose("NOTIFY_RCPT=%s\n",x);
            (void) snprintf(buf,sizeof(buf)-1,"RCPT TO: %s %s\r\n",
                            a->address,x);
        }
        else
        {
            (void) snprintf(buf,sizeof(buf)-1,"RCPT TO: <%s>\r\n",a->address);
        }

        showVerbose(">>> %s",buf);
        
        /* TODO: error checking */
        sockPuts(sfd,buf);
        if (smtpResponse(sfd) != 0)
        {
            smtpRset(sfd);
            return (-1);
        }
    }
    /*
    return (smtpResponse(sfd));
    */

    return (0);

}

/* SMTP: DATA */
static int smtpData(int sfd)
{
    sockPuts(sfd,"DATA\r\n");

    showVerbose(">>> DATA\r\n");

    return (smtpResponse(sfd));
}

/* SMTP: EOM */
int smtpEom(int sfd)
{
    sockPuts(sfd,"\r\n.\r\n");

    showVerbose("\r\n>>> .\r\n");

    return (smtpResponse(sfd));
}

void doCleanup(void)
{
    smtpDisconnect();
}

#ifdef WINNT
/*
** Handle Ctrl+C 
*/
static BOOL WINAPI CntrlHandler(DWORD CtrlEvent)
{
    break_out=0;

    switch (CtrlEvent)
    {
        case CTRL_C_EVENT:
        {
            break_out=1;
            (void) fprintf(stderr,"\nNot sending mail. Exiting.......\n");
            exit(1); /* XXXXXXXXXXXXXXXXXXXXXX */
            break;
        }
    }

    return (TRUE);
}
#endif /* WINNT */

/* SMTP: mail */
static int smtpMail(int sfd,char *to,char *cc,char *bcc,char *from,char *rrr,char *rt,
                    char *subject,char *attach_file,char *msg_body_file,
                    char *the_msg,int is_mime,int add_dateh)
{
    char
        *os="Unix",
        boundary[17],
        mbuf[1000];

    int
        newline_before;

    Sll
        *attachment_list;

#ifdef WINNT
    os="Windows";
#else
    os="Unix";
#endif /* WINNT */

    attachment_list=get_attachment_list();
    if (attachment_list)
    {
        is_mime=1;
    }

    if (subject)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"Subject: %s\r\n",subject);

        sockPuts(sfd,buf);

        showVerbose(buf);
    }

    /* headers */
    if (from)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"From: %s\r\n",from);
        sockPuts(sfd,buf);

        showVerbose(buf);
    }

    if (add_dateh)
    {
        /* add Date: header */
        char
            datebuf[65];

        memset(datebuf,0,sizeof(datebuf));
        if (rfc822_date(time(NULL),datebuf,sizeof(datebuf)-1) == 0)
        {
            memset(buf,0,sizeof(buf));
            (void) snprintf(buf,sizeof(buf)-1,"Date: %s\r\n",datebuf);
            sockPuts(sfd,buf);

            showVerbose(buf);
        }
    }


    if (to)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"To: %s\r\n",to);
        sockPuts(sfd,buf);

        showVerbose(buf);

    }

    if (cc)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"Cc: %s\r\n",cc);
        sockPuts(sfd,buf);

        showVerbose(buf);
    }

    /*
    if (bcc)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"Bcc: %s\r\n",bcc);
        sockPuts(sfd,buf);

        showVerbose(buf);
    }
    */

    if (rt != NULL)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"Reply-To: %s\r\n",rt);
        sockPuts(sfd,buf);
        showVerbose(buf);

    }
    if (rrr != NULL)
    {
        memset(buf,0,sizeof(buf));
        (void) snprintf(buf,sizeof(buf)-1,"Disposition-Notification-To: %s\r\n",rrr);
        sockPuts(sfd,buf);
        showVerbose(buf);
    }

    memset(buf,0,sizeof(buf));
    (void) snprintf(buf,sizeof(buf)-1,"X-Mailer: %s (%s)\r\n",MAILSEND_VERSION,os);
    sockPuts(sfd,buf);
    showVerbose(buf);

    memset(buf,0,sizeof(buf));
    (void) snprintf(buf,sizeof(buf)-1,"X-Copyright: %s\r\n",NO_SPAM_STATEMENT);
    sockPuts(sfd,buf);
    showVerbose(buf);

    /*
    if (is_mime && msg_file)
    */
    if (is_mime)
    {
        srand(time(NULL));
        memset(boundary,0,sizeof(boundary));
        mutilsGenerateMIMEBoundary(boundary,sizeof(boundary));
        (void) snprintf(buf,sizeof(buf)-1,"Content-type: multipart/mixed; boundary=\"%s\"\r\n",boundary);
        sockPuts(sfd,buf);
        showVerbose(buf);

        (void) snprintf(buf,sizeof(buf)-1,"Mime-version: 1.0\r\n");
        sockPuts(sfd,buf);
        showVerbose(buf);
    }

    sockPuts(sfd,"\r\n");
    showVerbose("\r\n");

    /*
    if (is_mime && msg_file)
    */
    if (is_mime)
    {
        char
            mime_tmpfile[BUFSIZ];

        FILE
            *tfp=NULL,
            *fp=NULL;
        int
            tfwd;

        /*
        ** If there a txt file or a one line messgae, attach them first
        */
        /* Part added by Smeeta Jalan -- starts */
        if (the_msg)
        {
            (void) snprintf(buf,sizeof(buf)-1,"\r\n--%s\r\n",boundary);
            sockPuts(sfd,buf);
            showVerbose(buf);

            (void) snprintf(buf,sizeof(buf)-1,"Content-Type: text/plain; charset=%s\r\n",g_charset);
            sockPuts(sfd,buf);
            showVerbose(buf);

            (void) strcpy(buf,"Content-Disposition: inline\r\n");
            sockPuts(sfd,buf);
            showVerbose(buf);

            sockPuts(sfd,"\r\n");
            showVerbose("\r\n");

            /* (void) strcpy(buf,the_msg); */
            mutilsSafeStrcpy(buf,the_msg,sizeof(buf)-1);
            sockPuts(sfd,buf);
            showVerbose(">>> %s",the_msg); 

            (void) snprintf(buf,sizeof(buf)-1,"\r\n\r\n");
            sockPuts(sfd,buf);
            showVerbose(buf);
        }

        if (msg_body_file)
        {  
            int
                rc;
            char
                mime_type[32],
                filename[1024];

            FILE
                *fp;

            rc=get_filepath_mimetype(msg_body_file,filename,
                    sizeof(filename)-1,mime_type,sizeof(mime_type)-1);
           /*
            * The file should not have binary characters it it.
            * It's user's responsibilty to make sure file is not binary.
            * If file is binary mail will have garbage as I'll not convert
            * the content to base64
            */
            fp=fopen(filename,"r");
            if (fp == (FILE *) NULL)
            {
                errorMsg("Could not open message body file: %s",filename);
                return (-1);
            } 

            (void) snprintf(buf,sizeof(buf)-1,"\r\n--%s\r\n",boundary);
            sockPuts(sfd,buf);
            showVerbose(buf);

            (void) snprintf(buf,sizeof(buf)-1,"Content-Type: %s; charset=%s\r\n",mime_type,g_charset);
            sockPuts(sfd,buf);
            showVerbose(buf);

            (void) strcpy(buf,"Content-Disposition: inline\r\n");
            sockPuts(sfd,buf);
            showVerbose(buf);

            sockPuts(sfd,"\r\n");
            showVerbose("\r\n");

            while (fgets(mbuf,sizeof(mbuf)-1,fp))
            {
                sockPuts(sfd,mbuf);
                showVerbose(">>> %s",mbuf); 
            }
            (void) fclose(fp);

            (void) snprintf(buf,sizeof(buf)-1,"\r\n\r\n");
            sockPuts(sfd,buf);
            showVerbose(buf);
        }
        /* Part added by Smeeta Jalan --ends --*/

        /* open a tmp file writing MIME content */
        (void) strcpy(mime_tmpfile,"./mailsendXXXXXX");

        /* handle mime */
        {
            Attachment
                *a;
            Sll
                *al;

            for (al=attachment_list; al; al=al->next)
            {
                a=(Attachment *) al->data;
                if (a == NULL)
                    continue;

                /* open the mime input file */
                fp=fopen(a->file_path,"rb");
                if (fp == (FILE *) NULL)
                {
                    errorMsg("%s (%d) - Could not open file for reading: %s (%s)",MFL,a->file_path,strerror(errno));
                    return (-1);
                }

                (void) strcpy(mime_tmpfile,"./mailsendXXXXXX");
#ifdef HAVE_MKSTEMP
                tfwd=mkstemp(mime_tmpfile);
                if (tfwd == -1)
                {
                    errorMsg("%s (%d) - Could not open tmp file \"%s\" for writing (for %s)\n",MFL,mime_tmpfile,a->file_name);
                    return (-1);
                }
                tfp=fdopen(tfwd,"w");
                if (tfp == NULL)
                {
                    (void) fprintf(stderr,"fdopen() failed on %s\n",mime_tmpfile);
                    close(tfwd);
                    unlink(mime_tmpfile);
                    return(-1);
                }
#else
                mktemp(mime_tmpfile);

                tfp=fopen(mime_tmpfile,"w");
                if (tfp == NULL)
                {
                    (void) fprintf(stderr,"Could not open tmp file \"%s\" for writing (for %s)\n",mime_tmpfile,a->file_name);
                    return (-1);
                }
#endif /* HAVE_MKSTEMP */

                /* base64 encode only if it is not mime type of text/plain */

                if (mutilsStrcasecmp(a->mime_type,"text/plain") != 0)
                {
                    mutilsBase64Encode(fp,tfp);
                    if (tfp)
                    {
                        (void) fclose(tfp);
                    }
                }
                else
                {
                    FILE
                        *ttp;
                    char
                        xbuf[BUFSIZ];

                    /* write the text file to tmp file */
                    ttp=fopen(a->file_path,"r");
                    if (ttp == NULL)
                    {
                        errorMsg("%s (%d) - could not open file %s for reading (%s)\n",MFL,a->file_path,ERR_STR);
                        continue;
                    }
                    while(fgets(xbuf,sizeof(xbuf)-1,ttp))
                    {
                        fputs(xbuf,tfp);
                    }
                    fclose(ttp);
                    (void) fclose(tfp);

                }
                if (fp)
                    (void) fclose(fp);

                (void) snprintf(buf,sizeof(buf)-1,"--%s\r\n",boundary);
                sockPuts(sfd,buf);
                showVerbose(buf);
                

                if (mutilsStrcasecmp(a->mime_type,"text/plain") == 0)
                {
                    (void) snprintf(buf,sizeof(buf)-1,"Content-Type: text/plain; charset=%s\r\n",g_charset);
                    sockPuts(sfd,buf);
                    showVerbose(buf);

                    (void) snprintf(buf,sizeof(buf)-1,"Content-Disposition: %s; filename=\"%s\"\r\n",
                                    a->content_disposition,
                                    a->file_name);
                    sockPuts(sfd,buf);
                    showVerbose(buf);

                    sockPuts(sfd,"\r\n");
                    showVerbose("\r\n");
                }
                else
                {
                    (void) snprintf(buf,sizeof(buf)-1,"Content-Type: %s; name=%s\r\n",a->mime_type,a->file_name);
                    sockPuts(sfd,buf);
                    showVerbose(buf);

                    (void) snprintf(buf,sizeof(buf)-1,"Content-Disposition: %s; filename=\"%s\"\r\n",
                                    a->content_disposition,
                                    a->file_name);
                    sockPuts(sfd,buf);
                    showVerbose(buf);

                    sockPuts(sfd,"Content-Transfer-Encoding: base64\r\n\r\n");
                    showVerbose("Content-Transfer-Encoding: base64\r\n\r\n");
                }

                fp=fopen(mime_tmpfile,"r");
                if (fp == (FILE *) NULL)
                {
                    (void) fprintf(stderr,"Could not open tmp file \"%s\" for reading\n",mime_tmpfile);
                    return (-1);
                }
                while (fgets(mbuf,sizeof(mbuf)-1,fp))
                {
                    sockPuts(sfd,mbuf);
                    showVerbose(">>> %s",mbuf); /* new line is there */
                }
                (void) fclose(fp);
                unlink(mime_tmpfile);
            }
            
            (void) snprintf(buf,sizeof(buf)-1,"\r\n--%s--\r\n",boundary);
            sockPuts(sfd,buf);
            showVerbose(buf);
        }
        goto done;
    }

    /* mail body */
    if (attach_file == NULL && the_msg == NULL) /* read from stdin */
    {

        /* if stdin is a terminal, print the instruction */
        if (isInConsole(_fileno(stdin)))
        {
            (void) printf("=========================================================================\n");
            (void) printf("Type . in a new line and press Enter to end the message, CTRL+C to abort\n");
            (void) printf("=========================================================================\n");
        }

#ifdef WINNT
        SetConsoleCtrlHandler(CntrlHandler,TRUE);
#endif /* WINNT */

        newline_before=1;
        while (fgets(mbuf,sizeof(mbuf)-1,stdin) && (break_out == 0))
        {
            if (newline_before && *mbuf == '.')
            {
                break;
            }
            else
            {
                int
                    len;
                /* vinicio qmail fix */
                len=strlen(mbuf);
                if (mbuf[len-1] != '\n')
                    strcat(mbuf,"\r\n");
                else
                {
                   mbuf[--len]='\0';
                   strcat(mbuf,"\r\n");
                }
                /* vinicio qmail fix */
                sockPuts(sfd,mbuf);
                showVerbose(">>> %s",mbuf);
            }
            newline_before=(*mbuf != '\0' && mbuf[strlen(mbuf)-1] == '\n');
            if (break_out == 1)
            {
                (void) fprintf(stderr," Breaking out\n");
                return (0);
            }
        }
    }
done:

    return (0);
}


/* It's stupid, I need to change all the args to a struct, one of those 
 * days! I'll do it
 */

/* returns 0 on success, -1 on failure */
int send_the_mail(char *from,char *to,char *cc,char *bcc,char *sub,
             char *smtp_server,int smtp_port,char *helo_domain,
             char *attach_file,char *txt_msg_file,char *the_msg,int is_mime,char *rrr,char *rt,
             int add_dateh)
{
    SOCKET
        sfd;

    TheMail
        *mail;

    Sll
        *al;

    int
        rc=(-1);

    /*
    (void) fprintf(stderr,"From: %s\n",from);
    (void) fprintf(stderr,"To: %s\n",to);
    (void) fprintf(stderr,"Cc: %s\n",cc);
    (void) fprintf(stderr,"Cc: %s\n",bcc);
    (void) fprintf(stderr,"Sub: %s\n",sub);
    (void) fprintf(stderr,"smtp: %s\n",smtp_server);
    (void) fprintf(stderr,"smtp port: %d\n",smtp_port);
    (void) fprintf(stderr,"domain: %s\n",helo_domain);
    (void) fprintf(stderr,"attach file: %s\n",attach_file);
    (void) fprintf(stderr,"txt_msg_file: %s\n",txt_msg_file);
    (void) fprintf(stderr,"the_msg: %s\n",the_msg);
    (void) fprintf(stderr,"is_mime: %d\n",is_mime);
    */

    al=getAddressList();

    if (al == (Sll *) NULL)
    {
        errorMsg("No To address/es specified");
        return (-1);
    }

    if (from == (char *) NULL)
    {
        errorMsg("No From address specified");
        return (-1);
    }

    if (smtp_server == (char *) NULL)
        smtp_server="127.0.0.1";

    if (smtp_port == -1)
        smtp_port=MAILSEND_SMTP_PORT;

    if (sub == (char *) NULL)
        sub=MAILSEND_DEF_SUB;

    if (helo_domain == (char *) NULL)
    {
        errorMsg("No domain specified");
        return (-1);
    }

    mail=newTheMail();
    if (mail == (TheMail *) NULL)
    {
        (void) fprintf(stderr,"Error: malloc failed in createTheMail()\n");
        return (-1);
    }

    /* open the network connection */
    sfd=smtpConnect(smtp_server,smtp_port);
    if (sfd == INVALID_SOCKET)
    {
        rc=(-1);
        goto cleanup;
    }

    rc=smtpHelo(sfd,helo_domain);
    rc=smtpMailFrom(sfd,from);
    rc=smtpRcptTo(sfd);
    rc=smtpData(sfd);

    rc=smtpMail(sfd,to,cc,bcc,from,rrr,rt,sub,attach_file,txt_msg_file,the_msg,is_mime,add_dateh);

    rc=smtpEom(sfd);
    rc=smtpQuit(sfd);

cleanup:
    return (rc);
}

