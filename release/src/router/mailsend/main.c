/*
**  main for mailsend - a simple mail sender via SMTP protocol
**
**  Limitations and Comments:
**      I needed to send a alert mail from a bare-bone networked NT machine,
**      but could not find a simple mail sender to do this (not surprised!).
**      so I wrote this one!
**
**
**  Development History:
**      who                  when           why
**      muquit@muquit.com    Mar-23-2001    first cut
*/

#define __MAIN__    1

#include "mailsend.h"


/* exits after writing the usage */
static void usage(void)
{
    char
        **p;

    static char
        *options[]=
        {
            "   -d    domain*          - domain name for SMTP Helo",
            "   -smtp hostname/IP*    - of the SMTP server",
            "   -p    SMTP port       - SMTP port",
            "   -t    to,to..*        - email address/es of the reciepient/s",
            "   -cc   cc,cc..         - Carbon copy address/es",
            "   +cc                   - don't ask for Carbon Copy",
            "   -bc   bcc,bcc..       - Blind carbon copy address/es",
            "   +bc                   - don't ask for Blind carbon copy",
            "   +D                    - don't add Date header",
            "   -f    address*        - email address of the sender",
            "   -sub  subject         - subject",
            "   -l    file            - a file containing the email addresses",
            "   -a    file,mime_type  - attach this file",
            "   -cs   character set   - for text/plain attachments (default is us-ascii)",
            "   -m    file,mime_type,[i/a] (i=inline,a=attachment)",
            "                         - attach the file (text,html etc) as body (inline)",
            "   -M    \"one line msg\"  - attach this one line text message",
            "   -v                    - verbose mode",
            "   -V                    - show version info",
            "   -w                    - wait for a CR after sending the mail",
            "   -rt  email_address    - add Reply-To header",
            "   -rrr email_address    - request read receipts to this address",
            "   -help                 - shows this help",
            (char *) NULL
        };
    (void) printf("\n");
    (void) printf("Version: %.1024s\n\n",MAILSEND_VERSION);
    (void) printf("Copyright: %.1024s\n\n",NO_SPAM_STATEMENT);

    (void) printf("usage: mailsend [options]\n");
    (void) printf("Where the options are:\n");

    for (p=options; *p != NULL; p++)
        (void) printf("%s\n",*p);

    (void) fprintf(stdout,"\nThe options with * must the specified\n");
    (void) fprintf(stdout,"Example (Note: type without newline):\n");
    (void) fprintf(stdout,
"\n"
" mailsend -f muquit@example.com -d example.com -smtp 10.100.30.1\n"
"  -t muquit@muquit.com -sub test -a \"file.txt,text/plain\"\n"
"  -a \"/usr/file.gif,image/gif\" -a \"file.jpeg,image/jpg\"\n\n");

(void) fprintf(stdout,
" mailsend -f muquit@example.com -d example.com -smtp 192.168.0.2\n"
"  -t muquit@muquit.com -sub test +cc +bc\n"
"  -a \"c:\\file.gif,image/gif\" -M \"Sending a GIF file\"\n\n");

(void) fprintf(stdout,
" mailsend -f muquit@example.com -d example.com -smtp 192.168.0.2\n"
"  -t muquit@muquit.com -sub test +cc +bc -cs \"ISO-8859-1\"\n"
"  -a \"file2.txt,text/plain\"\n\n");

    (void) fprintf(stdout,"Change content disposition to inline:\n");
    (void) fprintf(stdout,
" mailsend -f muquit@example.com -d example.com -smtp 10.100.30.1\n"
"  -t muquit@muquit.com -sub test -a \"nf.jpg,image/jpeg,i\"\n"
"  -M \"content disposition is inline\"\n\n");
    exit(0);
}

int main(int argc,char **argv)
{
    char
        *x,
        buf[BUFSIZ],
        *option;

    int
        is_mime=0,
        add_dateh=1,
        port=(-1),
        rc,
        no_cc=0,
        no_bcc=0,
        i;

    char
        *address_file=NULL,
        *helo_domain=NULL,
        *smtp_server=NULL,
        *attach_file=NULL,
        *msg_body_file=NULL,
        *the_msg=NULL,
        *to=NULL,
        *save_to=NULL,
        *save_cc=NULL,
        *save_bcc=NULL,
        *from=NULL,
        *sub=NULL,
        *cc=NULL,
        *bcc=NULL,
        *rt=NULL,
        *rrr=NULL;

    g_verbose=0;
    g_wait_for_cr=0;
    (void) strcpy(g_charset,"us-ascii");

    for  (i=1; i < argc; i++)
    {
        option=argv[i];
        switch (*(option+1))
        {

            case 'a':
            {
                if (strncmp("attach",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing file to attach");
                            return (1);
                        }
                        attach_file=argv[i];
                        add_attachment_to_list(attach_file);
                    }
                }

                break;
            }

            case 'b':
            {
                if (strncmp("bcc",option+1,2) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing BCc address/es");
                            return (1);
                        }
                        bcc=argv[i];
                        save_bcc=mutilsStrdup(bcc);

                        /* collapse all spaces to a comma */
                        mutilsSpacesToChar(bcc,',');
                        addAddressToList(bcc,"Bcc");
                    }
                    else if (*option == '+')
                    {
                        no_bcc=1;
                    }
                }
                else
                {
                    errorMsg("Unknown flag: %s\n",option);
                    return(1);
                }
                break;
            }


            case 'c':
            {
                if (strncmp("cc",option+1,2) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing Cc address/es");
                            return (1);
                        }
                        cc=argv[i];
                        save_cc=mutilsStrdup(cc);

                        /* collapse all spaces to a comma */
                        mutilsSpacesToChar(cc,',');
                        addAddressToList(cc,"Cc");
                    }
                    else if (*option == '+')
                    {
                        no_cc=1;
                    }
                }
                else if (strncmp("cs",option+1,2) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing character set");
                            return (1);
                        }
                        mutilsSafeStrcpy(g_charset,argv[i],sizeof(g_charset)-1);
                    }

                }
                else
                {
                    errorMsg("Unknown flag: %s\n",option);
                    return(1);
                }
                break;
            }

            case 'D':
            {
                if (strncmp("D",option+1,1) == 0)
                {
                    if (*option == '+')
                    {
                        add_dateh=0;
                    }
                }
                break;
            }

            case 'd':
            {
                if (strncmp("domain",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing domain name");
                            return (1);
                        }
                        helo_domain=argv[i];
                    }
                }
                break;
            }

            case 'f':
            {
                if (strncmp("from",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing From address/es");
                            return (1);
                        }
                        from=argv[i];
                    }
                }
                break;
            }

            case 'h':
            {
                if (strncmp("help",option+1,1) == 0)
                {
                    usage();
                }
                /* won't be here */
                break;
            }

            case 'l':
            {
                if (strncmp("list_address",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing address list file"); 
                            return (1);
                        }
                        address_file=argv[i];
                    }
                }
                break;
            }


            case 'p':
            {
                if (strncmp("port",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing SMTP Port");
                            return (1);
                        }
                        port=atoi(argv[i]);
                    }
                }
                break;
            }


            case 'm':
            {
                if (strncmp("msgbody",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing message body file");
                            return (1);
                        }
                        msg_body_file=argv[i];
                    }
                }
                break;
            }

            case 'M':
            {
                if (strncmp("Message",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing text message");
                            return (1);
                        }
                        the_msg=argv[i];
                    }
                }
                break;
            }

            case 's':
            {
                if (strncmp("smtp",option+1,3) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing smtp server");
                            return (1);
                        }
                        smtp_server=argv[i];
                    }
                }
                else if (strncmp("subject",option+1,3) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            errorMsg("Missing subject");
                            return (1);
                        }
                        sub=argv[i];
                    }
                }
                else
                {
                    errorMsg("Unknown flag: %s\n",option);
                    return(1);
                }

                break;
            }

            case 'v':
            {
                if (strncmp("verbose",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        g_verbose=1;
                    }
                }
                break;
            }

            case 'V':
            {
                (void) fprintf(stderr,"mailsend Version: %.1024s\n",MAILSEND_VERSION);
                exit(0);
                break;
            }

           case 't':
           {
                if (strncmp("to",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            (void) fprintf(stderr,"Error: missing to addresses\n");
                            return (1);
                        }
                        to=argv[i];
                        save_to=mutilsStrdup(to);
                        if (save_to == NULL)
                        {
                            errorMsg("memory allocation problem for -to");
                            return(-1);
                        }
                        save_to=fix_to(save_to);
                        to=fix_to(to);
                        /* collapse all spaces to a comma */
                        mutilsSpacesToChar(to,',');

                        /* add addresses to a singly linked list */
                        addAddressToList(to,"To");
                        /* Note: to is modifed now! */
                    }
                }
                break;
            }

            case 'r':
            {
                if (strncmp("rrr",option+1,3) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            (void) fprintf(stderr,"Error: missing to addresses for -rrr\n");
                            return (1);
                        }
                        rrr=mutilsStrdup(argv[i]);
                        if (rrr == NULL)
                        {
                            errorMsg("memory allocation problem for -rrr");
                            return(1);
                        }
                    }
                }
                else if (strncmp("rt",option+1,2) == 0)
                {
                    if (*option == '-')
                    {
                        i++;
                        if (i == argc)
                        {
                            (void) fprintf(stderr,"Error: missing to addresses for -rt\n");
                            return (1);
                        }
                        rt=mutilsStrdup(argv[i]);
                        if (rt == NULL)
                        {
                            errorMsg("memory allocation problem for -rt");
                            return(1);
                        }
                    }
                }
                else
                {
                    errorMsg("Unknown flag: %s\n",option);
                    return(1);
                }

                break;
            }

            case 'w':
            {
                if (strncmp("wait",option+1,1) == 0)
                {
                    if (*option == '-')
                    {
                        g_wait_for_cr=1;
                    }
                }

                break;
            }

            default:
            {
                (void) fprintf(stderr,"Error: Unrecognized option: %s\n",
                               option);
                return (1);
            }

        }
    }
    print_attachemtn_list();

    /*
    if (the_msg && txt_msg_file)
    {
        (void) fprintf(stderr,"\nError: -m and -M can not be used together\n");
        return(1);
    }
    */

    /*
    ** attaching a file or a one line message will make the mail a 
    ** MIME mail
    */
    if (attach_file || the_msg || msg_body_file)
    {
        is_mime=1;
    }

    if (smtp_server == NULL)
    {
        memset(buf,0,sizeof(buf));
        x=askFor(buf,sizeof(buf)-1,"SMTP server address/IP: ",EMPTY_NOT_OK);
        if (x)
            smtp_server=xStrdup(x);
    }

    if (helo_domain == NULL)
    {
        memset(buf,0,sizeof(buf));
        x=askFor(buf,sizeof(buf)-1,"Domain: ",EMPTY_NOT_OK);
        if (x)
            helo_domain=xStrdup(x);
    }

    if (from == NULL)
    {
        memset(buf,0,sizeof(buf));
        x=askFor(buf,sizeof(buf)-1,"From: ",EMPTY_NOT_OK);
        if (x)
            from=xStrdup(x);
    }



   /*
    ** The To address must be speicifed, even if the file with the list of
    ** addresses is specified. The specified To will be shown in the 
    ** envelope. None of the To, Cc and Bcc from the address list file will
    ** be shown anywhre.. that's how I like it. I hate long To, Cc or Bcc.
    ** muquit@muquit.com, Thu Mar 29 11:56:45 EST 2001 
    */

    if (save_to == NULL)
    {
        memset(buf,0,sizeof(buf));
        x=askFor(buf,sizeof(buf)-1,"To: ",EMPTY_NOT_OK);
        if (x)
        {
            save_to=xStrdup(x);
            addAddressToList(x,"To");
        }
    }

    /*
    ** if msg file specified, dont ask for unneeded things, as it could
    ** be used from other programs, and it will wait for input
    ** muquit@muquit.com Tue Apr 10 18:02:12 EST 2001 
    */

#ifdef WINNT
    if (attach_file == NULL && isInConsole(_fileno(stdin)))
#else
    if (attach_file == NULL && isatty(fileno(stdin)))
#endif /* WINNT */
    {
        if (save_cc == NULL && !no_cc)
        {
            memset(buf,0,sizeof(buf));
            x=askFor(buf,sizeof(buf)-1,"Carbon copy: ",EMPTY_OK);
            if (x)
            {
                save_cc=xStrdup(x);
                addAddressToList(x,"Cc");
            }
        }

        if (save_bcc == NULL && ! no_bcc)
        {
            memset(buf,0,sizeof(buf));
            x=askFor(buf,sizeof(buf)-1,"Blind Carbon copy: ",EMPTY_OK);
            if (x)
            {
                save_bcc=xStrdup(x);
                addAddressToList(x,"BCc");
            }
        }

        if (sub == NULL)
        {
            memset(buf,0,sizeof(buf));
            x=askFor(buf,sizeof(buf)-1,"Subject: ",EMPTY_OK);
            if (x)
                sub=xStrdup(x);
        }
    }

    /* if address file specified, add the addresses to the list as well */
    if (address_file != NULL)
        addAddressesFromFileToList(address_file);

    printAddressList();

    /* TODO: read from default file or registry */
    rc=validateMusts(from,save_to,smtp_server,helo_domain);
    if (rc == -1)
        return (1); /* exit */

#ifdef UNIX
    signal(SIGPIPE,SIG_IGN);
#endif /* UNIX */

    rc=send_the_mail(from,save_to,save_cc,save_bcc,sub,smtp_server,port,
                helo_domain,attach_file,msg_body_file,the_msg,is_mime,rrr,rt,add_dateh);

    if (rc == 0)
    {
        if (isInteractive())
        {
            (void) printf("Mail sent successfully\n");
            (void) fflush(stdout);
        }
    }
    else
    {
        if (isInteractive())
        {
            (void) printf("Could not send mail\n");
        }
    }

    if (isInteractive())
    {
        if (g_wait_for_cr)
        {
            printf("\nPress Enter to Exit: ");
            fgets(buf,sizeof(buf)-1,stdin);
        }
    }


    return (rc);
}
