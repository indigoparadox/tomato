/*
**  various utility routines for mailsend
**
**
**  Development History:
**      who                  when           why
**      muquit@muquit.com    Mar-26-2001    first cut
*/

#include "mailsend.h"
#define DAY_MIN     (24 * HOUR_MIN) /* minutes in a day */
#define HOUR_MIN    60      /* minutes in an hour */
#define MIN_SEC     60      /* seconds in a minute */


/*
** returns a positive number if the file descriptor is connected to
** console. 0 if not
*/
int isInConsole(int fd)
{
    int
        rc;
    rc=_isatty(fd);
    return (rc);
}

int isInteractive(void)
{
#ifdef WINNT
        if (isInConsole(_fileno(stdout)))
            return(1);
        else
            return(0);
#else
        if (isatty(fileno(stdout)))
            return(1);
        else
            return(0);
#endif /* ! WINNT */
 
    return(0);

}
/*
** duplicate a string. exits on failure
*/
char *xStrdup (char *string)
{
    char
        *tmp;

    if (string == (char *) NULL || *string == '\0')
        return ((char *) NULL);

    /* malloc twice as much memory.  */
    tmp = (char *) malloc ((int) strlen(string)+1);

    if (tmp == (char *) NULL)
    {
        (void) fprintf(stderr,"Error: mystrdup(): memory allocation problem\n");
        exit(0);
    }
    /* it's safe to copy this way */
    (void) strcpy(tmp, string);
    return (tmp);
}

void errorMsg(char *format,...)
{
    va_list
        args;

    va_start(args,format);
    (void) fprintf (stderr,"Error: ");
    vfprintf(stderr,format,args);
    (void) fprintf(stderr,"\n");
    va_end(args);
}

void showVerbose(char *format,...)
{
    va_list
        args;

    if (g_verbose == 1)
    {
        va_start(args,format);
        vfprintf(stdout,format,args);
        (void) fflush(stdout);
        va_end(args);
    }

}




/*
** Initialize TheMail structure. returns pointer to the TheMail sturcture on
** success, NULL on failure.
*/
TheMail *newTheMail(void)
{
    TheMail
        *the_mail=(TheMail *) NULL;

    the_mail=(TheMail *) malloc(sizeof(TheMail));
    if (the_mail == (TheMail *) NULL)
    {
        (void) fprintf(stderr,"initTheMail(): malloc failed\n");
        return ((TheMail *) NULL);
    }
    memset(the_mail,0,sizeof(TheMail));

    return (the_mail);
}

Address *newAddress(void)
{
    Address
        *a;

    a=(Address *) malloc(sizeof(Address));
    if (a == (Address *) NULL)
    {
        (void) fprintf(stderr," newAddress() malloc failed\n");
    }
    memset(a,0,sizeof(Address));

    return (a);
}


int validateMusts(char *from,char *to,char *smtp_server,char *helo_domain)
{
    int
        err=0;

    if (to == (char *) NULL)
    {
        errorMsg("No To address/es specified. Speicfy with: -t to,to,..");
        err++;
    }

    if (from == (char *) NULL)
    {
        errorMsg("No From address specified. Specify with: -f");
        err++;
    }


    if (smtp_server == (char *) NULL)
    {
        errorMsg("No SMTP server address or IP specified. Specify with -smtp");
        err++;
    }


    if (helo_domain == (char *) NULL)
    {
        errorMsg("No domain specified. Specify with: -d");
        err++;
    }

    if (err)
        return (-1);

    return (0);
}

char *askFor(char *buf,int buflen,char *label,int ask)
{

    if (label == NULL || *label == '\0')
        return (NULL);

again:
    if (isInConsole(_fileno(stdin)))
    {
        (void) fprintf(stdout,label);
        (void) fflush(stdout);
        (void) fflush(stderr);
    }

    (void) fgets(buf,buflen,stdin);
    if (*buf == '\0' || *buf == '\n')
    {
        if (ask == EMPTY_NOT_OK)
            goto again;
    }

    mutilsChopNL(buf);

    return (buf);
}

/*
** remote mailto from To
** remote mailto: from the address
**  mailto:foo => foo
** mailto: => mailto:
** mailto:x => "x"
** mailto: x => " x"
*/
char *fix_to(char *to)
{
    char
        *p=to;

    if (to == NULL || *to == '\0')
        return(p);

    /* mailto:foo */
    if (strlen(to) > 7 && mutilsStrncasecmp(to,"mailto:",7) == 0)
    {
        p=to;
        p=p+7;
        return(p);
    }

    return(p);
}

/**
 * return filepath and mime_type
 * @param str   String of the form "file,mime_type" 
 *              example: "c:\usr\local\foo.txt,text/plain"
 *              "/usr/local/foo.html,text/html"
 * @param filepath returns
 * @param fp_size  size of the buffer filepath
 * @param mime_type returns
 * @param mt_size size of the buffer mime_type
 *
 * @return 0 on success, -1 otherwise
 * 
 * Note: it will not expand "~ $" etc in the filepath
 * This function is used when a body is attached with -m flag
 *              
 */
int get_filepath_mimetype(char *str,char *filepath,int fp_size,char *mype_type,int mt_size)
{

    int
        rc=0;
    char
        *fp,
        *mt;
    if ((mt=strchr(str,ATTACHMENT_SEP)))
    {
        *mt++='\0';
    }
    else
    {
        errorMsg("Could not determine mime-type from input: %s\n",str);
        return(-1);
    }
    mutilsSafeStrcpy(mype_type,mt,mt_size);

    /* get the filepath out */
    fp=str;
    mutilsSafeStrcpy(filepath,fp,fp_size);
    return(rc);
}

/**
 * Calculate Date for RFC822 Date: header
 *
 * @param when     time since unix epoch
 * @param datebuf  returns NULL terminated date string.
 *                  Example: Wed, 17 May 2006 13:55:35 -0400
 * @param bufsiz   size of buffer dates. It's callers responsibily to make sure 
 *                 datebuf contains enough space. It must be at least 64 bytes. 
 *                 Usually it'll contain 31 bytes.
 *
 * returns 0 on success, -1 on failure
 *
 * The code is adapted from postfix. 
 *
 * Changes I made:
 *  - I use fixed size buffers instead of dynamic ones.
 *  - I don't use %e to calcuate day of the week, instead I use %d. Windows strftime
 *    does not have %e.
 *  - I don't add timzone name.
 *
 * Reference: http://cr.yp.to/immhf/date.html
 */
int rfc822_date(time_t when,char *datebuf,int bufsiz)
{
    char
        ts1[32],
        ts2[32];

    struct tm
        *lt;

    struct tm
        gmt;

    int
        gmtoff;

    if (bufsiz < 64)
    {
        (void) fprintf(stderr,"buffer size of date must be > 31, it is %d\n",bufsiz);
        return(-1);
    }

    memset(datebuf,0,bufsiz);
    memset(ts1,0,sizeof(ts1));
    memset(ts2,0,sizeof(ts2));

    gmt=*gmtime(&when);
    lt=localtime(&when);

    gmtoff = (lt->tm_hour - gmt.tm_hour) * HOUR_MIN + lt->tm_min - gmt.tm_min;
    if (lt->tm_year < gmt.tm_year)
    gmtoff -= DAY_MIN;
    else if (lt->tm_year > gmt.tm_year)
    gmtoff += DAY_MIN;
    else if (lt->tm_yday < gmt.tm_yday)
    gmtoff -= DAY_MIN;
    else if (lt->tm_yday > gmt.tm_yday)
    gmtoff += DAY_MIN;
    if (lt->tm_sec <= gmt.tm_sec - MIN_SEC)
    gmtoff -= 1;
    else if (lt->tm_sec >= gmt.tm_sec + MIN_SEC)
    gmtoff += 1;

    /*
    ** windows strftime does not have %e, so I'll use %d instead.
    ** day 1 - 9 can be written as 01 - 09 
    */
    (void) strftime(ts1,sizeof(ts1)-1,"%a, %d %b %Y %H:%M:%S ",lt);
    if (gmtoff < -DAY_MIN || gmtoff > DAY_MIN)
    {
        (void) fprintf(stderr,"UTC time offset %d is larger than one day",gmtoff);
        return(-1);
    }

    (void) snprintf(ts2,sizeof(ts2)-1,"%+03d%02d",
            (int) (gmtoff / HOUR_MIN),(int) (abs(gmtoff) % HOUR_MIN));

    /* put everything in the buffer. it's usually 31 bytes */
    (void) snprintf(datebuf,bufsiz,"%s%s",ts1,ts2);

    /* I will not add timezone name, it's not required */

    return(0);
}

