#ifndef MAILSEND_H
#define MAILSEND_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>

#include "mutils.h"
#include "msock.h"
#include "sll.h"

#ifdef UNIX
#include <signal.h>
#endif /* UNIX */

/*
**  header for mailsend - a simple mail sender via SMTP
**  $Id: mailsend.h,v 1.3 2002/06/22 21:17:29 muquit Exp $
**
**  Development History:
**      who                  when           why
**      muquit@muquit.com    Mar-23-2001    first cut
*/

#define MFL __FILE__,__LINE__

#define MAILSEND_VERSION    "@(#) mailsend v1.14"
#define MAILSEND_PROG       "mailsend"
#define MAILSEND_AUTHOR     "muquit@muquit.com"
#define MAILSEND_URL        "http://www.muquit.com/"
#define NO_SPAM_STATEMENT   "GNU GPL. It is illegal to use this software for Spamming"

#define MAILSEND_SMTP_PORT  25
#define MAILSEND_DEF_SUB    ""

#define A_SPACE ' '

#define EMPTY_OK        0x01
#define EMPTY_NOT_OK    0x02
#define ATTACHMENT_SEP  ','


#ifdef EXTERN
#undef EXTERN
#endif /* EXTERN */

#ifndef __MAIN__
#define EXTERN extern
#else 
#define EXTERN
#endif /* __MAIN__ */

#ifdef WINNT
#define snprintf _snprintf
#endif /* WINNT */

#define CFL __FILE__,__LINE__

#define CHECK_MALLOC(x) \
do \
{ \
    if (x == NULL) \
    { \
        (void) fprintf(stderr,"%s (%d) - Memory allocation failed\n",CFL); \
        exit(0); \
    }\
}while(0)

#define ERR_STR strerror(errno)

EXTERN int  g_verbose;
EXTERN int  g_wait_for_cr;
EXTERN char g_charset[33];

typedef struct _Address
{

    /*
    ** label holds strings like "To" "Cc" "Bcc". 
    ** The address is the email address.
    */

    char
        *label,     /* To: Cc: Bcc: */
        *address;   /* the email address */
}Address;

typedef struct _Attachment
{
    char
        *file_path,
        *file_name;
    char
        *mime_type;
    char
        *content_disposition;
}Attachment;

/* the mail sturct */
typedef struct _TheMail
{
    SOCKET
        fd;

    Address
        *address;

    char
        *from,
        *subject,
        *x_mailer,
        *smtp_server,
        *helo_domain,
        *msg_file;
} TheMail;


/* struct for $HOME/.mailsendrc */
typedef struct _Mailsendrc
{
    char
        *domain,
        *from,
        *smtp_server;
}Mailsendrc;

/* function prototypes */
char        *xStrdup(char *string);
int         addAddressToList(char *a,char *label);
TheMail     *initTheMail(void);
Address     *newAddress(void);
Sll         *getAddressList(void);
void        printAddressList(void);
int         send_the_mail(char *from,char *to,char *cc,char *bcc,char *sub,
                     char *smtp_server,int smtp_port,char *helo_domain,
                     char *attach_file,char *txt_msg_file,char *the_msg,
                     int is_mime,char *rrr,char *rt,int add_dateh);
TheMail     *newTheMail(void);
void        errorMsg(char *format,...);
void        showVerbose(char *format,...);
int         addAddressesFromFileToList(char *adress_list_file);
int         validateMusts(char *from,char *to,char *smtp_server,
                          char *helo_domain);
char        *askFor(char *buf,int buflen,char *label,int loop);
int         isInConsole(int fd);
int         add_attachment_to_list(char *file_path_mime);
Sll         *get_attachment_list();
void        print_attachemtn_list();
char        *fix_to(char *to);
int         isInteractive(void);
int         get_filepath_mimetype(char *str,char *filename,int fn_size,
                                  char *mype_type,int mt_size);
int         rfc822_date(time_t when,char *datebuf,int bufsiz);

#endif /* ! MAIL_SEND_H */
