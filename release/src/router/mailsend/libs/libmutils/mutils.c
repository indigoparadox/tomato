/* general utility routines */

#include <mutils.h>

/* file descriptor dot lock file */
static int 
    lock_fd=(-1);


/*
**  mutilsHowmanyCommas()
**  calculates how many commas will be added to the buffer
**
**  RCS
**      $Revision: 1 $
**      $Date: 2/24/04 8:38p $
**  Return Values:
**      no of commas 
**
**  Parameters:
**      buf     buf to scan
**
**  Side Effects:
**      none
**
**  Limitations and Comments:
**      buf must be a initialized string (at least)
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Oct-18-1997    first cut
*/
int mutilsHowmanyCommas(char *buf)
{
    int
        n=0,
        length;

    if (*buf == '\0')
        return(0);

    length=strlen(buf);

    if ((length % 3) == 0)
        n=(length/3)-1;
    else
        n=length/3;

    if ( n < 0)
        n=0;

    return(n);
}

/*
**  mutilsCommaize()
**  add a comma after every 3rd digit from right
**
**  RCS
**      $Revision: 1 $
**      $Date: 2/24/04 8:38p $
**  Return Values:
**      none
**
**  Parameters:
**      buf     buf to modify
**
**  Side Effects:
**      buf is modified
**
**  Limitations and Comments:
**      buf must have enough space to hold the extra , characters
**
**  Development History:
**      who                  when       why
**      ma_muquit@fccc.edu   no idea    first cut
**                           Oct-18-97  added dynamic buffer 
**                           May-26-1999 rewrote as the old version has
**                           memory overstepping bug. Code adapted from
**                           C-Snippets, file commaflt.c by Bruce Wedding and
**                           and Kurt Kuzba
*/
void mutilsCommaize(char *buf)
{
    char
        *pbuf=(char *) NULL;

    int
        bf=0,
        cm=0,
        tm=0;

    if (*buf != '\0')
    {
        pbuf=mutilsStrdup(buf);
        if (pbuf == (char *) NULL)
            return; /* malloc failed, return quitely */

        mutilsReverseString(pbuf);
        while ((buf[bf++]=pbuf[tm++]) != 0)
        {
            if(++cm % 3 == 0 && pbuf[tm])
                buf[bf++]=',';
        }
        if (pbuf)
            (void) free((char *) pbuf);

        mutilsReverseString(buf);
    }
}


/*
** NULL terminate the buffer at the first sight of a non-digit character
*/

/*
**  mutilsCleanBuf()
*   NULL terminate the buffer at the first sight of a non-digit character
**
**  Parameters:
**  char    *buf            the buffer to clean
**  char    *bytes_in_buf   the buffer size
**  int     *length         returns, bytes in buffer after cleaning
**
**  Return Values:
**
**
**  Limitations and Comments:
**  buf is modified
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-31-1999    first cut
*/
void mutilsCleanBuf(buf,bytes_in_buf,length)
char
    *buf;
int
    bytes_in_buf;
int
    *length;
{
    int
        i;


    int
        c=0;
    for (i=0; i < bytes_in_buf; i++)
    {
        if (!isdigit(buf[i]))
        {
            buf[i]='\0';
            break;
        }
        c++;
    }

    *length=c;
}

/*
**  mutilsParseURL()
**  parse a URL of the form
**          http://host:port/thepage.html
**          http://host/thepage.html
**
**  Parameters:
**  char *url               the url to parse
**  char *hostname          the hostname (returns)
**  int  hostname_len       allocated space length in hostname
**  int  *port              the port (returns)
**  char *page              the page (returns)
**  int page_len            the alloacated space length in page
**
**  Return Values:
**  0 on success
**  -1 on failure
**
**  Limitations and Comments:
**  - not much error checking. 
**  - hostname and page must have enough space preallocated by the caller.
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-10-1999    first cut
**                           Jul-20-1999    handle buffer overflow
*/
int mutilsParseURL(url,hostname,hostname_len,port,page,page_len)
char
    *url;

char
    *hostname;

int
    hostname_len;

int
    *port;

char
    *page;

int
    page_len;

{
    char
        tmpbuf[32],
        *q,
        *r,
        *p,
        *ptmp;

    int
        tmpbuf_len,
        hlen=0,
        tlen=0;

    if (url == NULL || *url == '\0')
        return(-1);

    /* initialize */
    *port=80;
    *hostname='\0';
    *page='\0';
    *tmpbuf='\0';

    tmpbuf_len=sizeof(tmpbuf);

    ptmp=mutilsStrdup(url);
    if (!ptmp)
        return (-1);
    if (strlen(ptmp) < 7)
    {
        (void) free(ptmp);
        return (-1);
    }

    /* skip http:// */
    if ( (ptmp[0] == 'h' || ptmp[0] == 'H') &&
         (ptmp[1] == 't' || ptmp[1] == 'T') &&
         (ptmp[2] == 't' || ptmp[2] == 'T') &&
         (ptmp[3] == 'p' || ptmp[3] == 'P') &&
         (ptmp[4] == ':') &&
         (ptmp[5] == '/') &&
         (ptmp[6] == '/'))
    {
        p = ptmp+7;
    }
    else
    {
        (void) free(ptmp);
        return (-1);
    }

    hlen=0;
    tlen=0;
    q=hostname;
    r=tmpbuf;
    for (; (*p != '/') && (*p != '\0'); p++)
    {
        /* get host part out */
        if (*p != ':' && !isdigit(*p))
        {
            if (++hlen > hostname_len-1)
            {
                continue;
            }
            *q++ = *p;
        }

        /* get port out if any */
        if (isdigit(*p))
        {
            if (++tlen > tmpbuf_len-1)
                continue;
            *r++ = *p;
        }
    }
    /* NULL terminate */
    *q='\0';
    *r='\0';


    if (*tmpbuf != '\0')
        *port=atoi(tmpbuf);

    /* the rest is the page */
    if (*p == '\0')
        return (-1);

    (void) mutilsStrncpy(page,p,page_len);

    (void) free(ptmp);
    return (0);
}



/*
**  mutilsSpacesToChar()
**      converts all spaces to a single character
**
**  Parameters:
**      char    *str    - spaces to collapse from
**      int     c       - to this character
**
**  Return Values:
**      pointer to the string
**
**  Limitations and Comments:
**      str is modified.
**
** adapted from C snipptes lib: lv1ws 
**
**  Development History:
**      who                  when           why
**      muquit@lucent.com    Mar-27-2001    first cut
*/
char *mutilsSpacesToChar(char *str,int c)
{
    char
        *ibuf,
        *obuf,
        *nbuf;

    register int
        i,
        n;

    if (str)
    {
        ibuf=obuf=str;
        i=n=0;

        while (*ibuf)
        {
            if (isspace(*ibuf) && n)
                ibuf++;
            else
            {
                if (!isspace(*ibuf))
                    n=0;
                else
                {
                    *ibuf=c;
                    n=1;
                }
                obuf[i++] = *ibuf++;
            }
        }
        obuf[i]='\0';

    }
    return (str);
}

/*
**  mutilsRmallws()
**  remove all whitespace (leading or trailing) from a string.
**
**  Parameters:
**  char    *str
**
**  Return Values:
**  pointer to the string
**
**  Limitations and Comments:
**  str is modified. borrowed from public domain c snippets libraray.
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-31-1999    first cut
*/
char *mutilsRmallws(char *str)
{
    char
        *obuf,
        *nbuf;

    if (str)
    {
        for (obuf=str, nbuf=str; *obuf; ++obuf)
        {
            if (!isspace(*obuf))
                *nbuf++ = *obuf;
        }
        *nbuf='\0';
    }
    return (str);
}
/*
**  mutilsStristr()
**  case insensitive version of ANSI strstr()
**
**  Parameters:
**  char    *s  hay stack
**  char    *t  needle
**
**  Return Values:
**  pointer to hay stack  where the needle is found
**  NULL if not found
**
**  Limitations and Comments:
**  from swish package by kevin h , kevin called it lstrstr()
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-31-1999    first cut
*/
char *mutilsStristr(s,t)
char
    *s;
char
    *t;
{

int
    i,
    j,
    k,
    l;

    for (i = 0; s[i]; i++)
    {
        for (j = 0, l = k = i; s[k] && t[j] &&
            tolower(s[k]) == tolower(t[j]); j++, k++)
            ;
        if (t[j] == '\0')
            return s + l;
    }
    return NULL;
}

/*
**  mutilsIsinname()
**  checks if the mask fits in the string
**
**  RCS
**      $Revision: 1 $
**      $Date: 2/24/04 8:38p $
**  Return Values:
**      1       if fits
**      0       if not
**      -1      if there's a memory allocation problem
**
**  Parameters:
**      string      source string
**      mask        mask string
**
**  Example:
**      www.fccc.edu  *.fccc.edu        fits
**      www.fccc.edu  www*              fits
**      www.fccc.edu  *fccc*            fits
**      132.138.4.6   132*              fits
**      etc.....
**
**  Side Effects:
**      none
**
**  Limitations and Comments:
**      borrowed from swish by Kevin H
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Oct-18-1997    first cut
*/
int mutilsIsinname(string,mask)
char
    *string;
char
    *mask;
{
    int
        i,
        j;

    char
        firstchar,
        lastchar,
        *tempmask;

    if ((*string == '\0') || (*mask == '\0'))
        return (0); /* mm */

    if (!strcmp(mask, "*"))
            return 1;

    firstchar=mask[0];
    lastchar=mask[(strlen(mask) - 1)];
    tempmask=(char *) malloc(strlen(mask)*sizeof(char)+1);
    if (tempmask == NULL)
    {
        (void) fprintf(stderr,"libmutils.a-mutilsIsinname(), malloc failed\n");
        return(-1);
    }

    for (i = j = 0; mask[i]; i++)
        if (mask[i] != '*')
            tempmask[j++] = mask[i];

    tempmask[j]='\0';
    if (firstchar == '*')
    {
        if (lastchar == '*')
        {
            if ((char *) mutilsStristr(string, tempmask))
            {
                free(tempmask);
                return 1;
            }
        }
        else
        {
            if ((char *) mutilsStristr(string, tempmask) ==
                 string + strlen(string) - strlen(tempmask))
            {
                free(tempmask);
                return 1;
            }
        }
    }
    else if (lastchar == '*')
    {
        if ((char *) mutilsStristr(string, tempmask) == string)
        {
            free(tempmask);
            return 1;
        }
    }
    else
    {
        /*
        ** changed from strcmp(), patch sent my
        ** Takeshi OKURA <okura@osa.ncs.co.jp>
        ** Oct-30-1997
        */
        if (!mutilsStrcasecmp(string, tempmask))
        {
            free(tempmask);
            return 1;
        }
    }
    free(tempmask);

    return 0;
}
/*
**  mutilsGetTIme()
**  get current time, ex: Wed Jun 30 21:49:08 1993
**
**  Parameters:
**  none
**
**  Return Values:
**  pointer a string containing time
**
**  Limitations and Comments:
**  remove the trailing new line from ctime(). subsequent call will
**  destroy the buffer. ctime() retuns pointer to a static buffer.
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-31-1999    first cut
*/
char *mutilsGetTime ()
{
    time_t
        tm;

    char
        *times;

    tm=time (NULL);
    times=ctime(&tm);
    times[(int) strlen(times)-1] = '\0';
    return (times);
}   

/* chop new line from a string */
/* string is modifiled */
char mutilsChopNL(string)
char
    *string;
{
    char
        c;
    char
        *ptr;

    c='\0';

    if (string == NULL || *string == '\0')
        return(c);

    for (ptr = string; *ptr; ptr++);

    if (ptr != string)
    {
        c = *(--ptr);

        if (c == '\n')
        {
            *ptr='\0';
        }
    }

    return (c);
}

/*
 *  mutilsStripLeadingSpace () -   strips leading space/s from a string
 *
 *  Description:
 *      This function strips leading spaces from a string and
 *      null terminates it.
 *
 *  Input Parameters:
 *      char    *str
 *
 *  Output Parameters:
 *      char    *str
 *
 *  Return Values:
 *      one
 *
 *  Side Effects:
 *      str is modified
 *
 *  Limitations and Comments:
 *      str must points to a pre-allocated static or dynamic space
 *
 *  Development History:
 *      who                  when       why
 *      MA_Muquit@fccc.edu   18-Jun-96  first cut
 */
void mutilsStripLeadingSpace(char *s)
{
    int i,
        n=0;

    char
        *ls;

    if (s == NULL || *s == '\0')
        return;

    for (i=0; i < (int) strlen(s); i++)
    {
        if ((s[i] == ' ' ) || (s[i] == '\t'))
            n++;
        else
            break;
    }

    if (n)
    {
        ls= (char *) malloc(strlen(s)*sizeof(char)+1);
        if (ls != (char *) NULL)
        {
           (void) strcpy (ls,s+n);
           (void) strcpy (s,ls);
           (void) free ((char *) ls);
        }
    }
}

/*
**  mutilsStripTrailingSpace() 
**  strips trailing spaces at the end of a string.
**
**  Parameters:
**  char *str   the string
**
**  Return Values:
**  none
**
**  Limitations and Comments:
**  the string is modified
**
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Jul-04-1998    first cut
*/
void mutilsStripTrailingSpace (char *str)
{
    register int
        i;

    if (str == NULL || *str == '\0')
        return;
    for ((i=(int) strlen(str)-1); i >= 0; i--)
    {
        if ((str[i] == ' ') || (str[i] == '\t'))
        {
            str[i] = '\0';
        }
        else
            break;
    }
}

/*
** make a temporary filename 
** filename - returns
*/
int mutilsTmpFilename(char *filename)
{
#define TMP_DIR      "/tmp"
#define TMP_TEMPLATE "%s/editdb.XXXXXX"    
    char
        *dir;

    dir=(char *) getenv("TMPDIR");
    if (dir == (char *) NULL)
        dir=TMP_DIR;

    (void) sprintf(filename,TMP_TEMPLATE,dir);
#ifdef HAVE_MKSTEMP
    return(mkstemp(filename));
#else
    return(mktemp(filename));
#endif /* ! HAVE_MKSTEMP */
}


/*
 *  basename()  -   return a file's basename
 *
 *  RCS:
 *      $Revision: 1 $
 *      $Date: 2/24/04 8:38p $
 *
 *  Security:
 *      Unclassified
 *
 *  Description:
 *      Basename parses a char string containing a path name and
 *      returns a pointer to the file's base name.
 *
 *  Input Parameters:
 *      char *path  -   the fully qualified path to parse
 *
 *  Output Parameters:
 *      None.
 *
 *  Return Values:
 *      (char *)    -   a pointer to the start of the base name within
 *                      'path'
 *
 *  Side Effects:
 *      None.
 *
 *  Limitations and Comments:
 *      Basename handles both DOS and UNIX path separators; it also
 *      handles the DOS device delimiter.
 *      
 *      I renamed it to mutilsBasename() as basename() will have collision
 *      now a days. -- mm, augh-11-1999
 *
 *      Note that this function can return NULL.
 *
 *  Development History:
 *      3/26/92, jps, first cut
 */
char *mutilsBasename(path)
char
    *path;
{
    char
        *cptr;

    for (cptr = path + strlen(path); cptr >= path; --cptr)
    {
        switch (*cptr)
        {
            case ':':
            case '/':
            case '\\':
                return ++cptr;
        }
    }
    return (path);
}


/*
**  mutilsDotLock()
**  open a file for locking purpose. If the system is Unix, use
**  Kernel lock on the file. 
**
**  Parameters:
**  char *lockfile_path  - full path of the lock file
**  char *errbuf         - feels this errbuf in case of error
**
**  Return Values:
**  none
**
**  Limitations and Comments:
**  if you're on Unix, make sure the partition is not NFS mounted, if i
**  is NFS mounted, the counter might hang.
**  errbuf must have enough space to hold the message
**
**  Development History:
**      who                  when           why
**      ma_muquit@fccc.edu   Aug-16-1999    first cut
*/

static char
    s_lockfile[BUFSIZ];

void mutilsDotLock(char *filepath,char *errbuf)
{
    *errbuf='\0';
    *s_lockfile='\0';

#ifdef SYS_WIN32
        lock_fd=sopen(filepath,_O_RDWR|_O_CREAT,SH_DENYWR,_S_IREAD|_S_IWRITE);
#else
        lock_fd=open(filepath,O_RDWR|O_CREAT,0644);
#endif

    if (lock_fd < 0) /* open failed */
    {
        (void) sprintf(errbuf,"Could open counter database lock file for writing:\n%s",filepath);
        return;
    }
    (void) strcpy(s_lockfile,filepath);

#ifdef SYS_UNIX
        mutilsSetLock(lock_fd);
#endif

}


/* unlock the file */
void mutilsDotUnlock(int delete)
{
    if (lock_fd >= 0)
    {
        /* closing also unlocks kernel locking */
        (void) close(lock_fd);
        lock_fd=(-1);
        if (delete == 1)
            unlink(s_lockfile);
    }
}


#ifdef SYS_UNIX
void mutilsSetLock (int fd)
{
#ifdef HAVE_FLOCK
    (void) flock(fd,LOCK_EX);
#else
    lseek(fd,0L,0);
    (void) lockf(fd,F_LOCK,0L);
#endif
}
    
void mutilsUnsetLock (int fd)
{
#ifdef HAVE_FLOCK 
    (void) flock(fd,LOCK_UN);
#else
    lseek(fd,0L,0);
    (void) lockf(fd,F_ULOCK,0L);
#endif
}   
#endif  /* SYS_UNIX */


/*
** mutilsWhich()
** returns 0 if the program is found in path
**         -1 otherwise
**
** modified from: /usr3/doc/which-1.0/which.c on RH 5.2
*/

int mutilsWhich(char *name)
{
#ifndef X_OK
#define X_OK    0x01
#endif /* X_OK */

    char
        s,
        *p,
        *path,
        szbuf[BUFSIZ];
    int
        found=0,
        len;

    /* check if the program exists and executable */
    if (access(name,X_OK) == 0)   /* found it */
        return(0);

    /* go through the PATH */
    path=getenv("PATH");
    if (path == NULL)
        return(-1);

    p=path;
    found=0;
    while (*p != '\0' && found == 0)
    {
        len=0;
        while (*p != ':' && *p != '\0')
        {
            len++;
            p++;
        }
        s=(char) *p;
        *p='\0';
        (void) sprintf(szbuf,"%s/%s",p-len,name);
        *p=s;
        if (*p)
            p++;

        if (access(szbuf,X_OK) == 0)
            found=1;
    }
    if (found == 0)
        found=(-1);

    if (found == 1) /* found */
        found=0;

    return(found);
}

/* eat white space from a open FILE pointer */
/* return -1 if fp is NULL */
int mutilsEatWhitespace(FILE *fp)
{
    int
        c;

    if (fp == NULL)
        return(-1);

    for(c =getc(fp);isspace(c) && ('\n' != c); c=getc(fp))
        ;
    return (c);

}

/* eat comments space from a open FILE pointer */
/* this function is called after seeing the comment character, so the */
/* function actually eats the line till sees a new line */
/* return -1 if fp is NULL */
int mutilsEatComment(FILE *fp)
{
    int
        c;

    if (fp == NULL)
        return(-1);

    for(c=getc(fp); ('\n' != c) && (EOF != c ) && (c > 0); c=getc(fp))
            ;

    return(c);
}

/*
** mutilsGgetDirname()
**   returns the base directory of file (Unix or Windows)
**   if fails returns NULL
**  Note: returns pointer to a malloc'd space, caller is responsible to
**        free it. 
*/
char *mutilsGetDirname(file)
char
    *file;
{
    char
        *f,
        *p;

    int
        len;

    if (file == NULL || *file == '\0')
        return (NULL);

    f=strdup(file);
    if (f == NULL)
        return(NULL);

    if ((p=strrchr(f,'/')) || (p=strrchr(f,'\\')))
    {
        *p='\0';
        if (f)
            return(f);
    }
    else
    {
        p=strdup("./");
        return(p);
    }
    return (NULL);
}

/**
 * @brief   Free memory associated with tokens
 * @param tokens    The tokens to free
 * @param ntokens   Number of tokens in tokens
 */
void mutilsFreeTokens(char **tokens,int ntokens)
{
    int
        i;

    /* free memory allocated for each token */
    for (i=0; i < ntokens; i++)
    {
        if (tokens[i])
            (void) free((char *) tokens[i]);
    }

    /* free the tokens itself */
    if (tokens)
        (void) free((char *) tokens);
}

/**
 * @brief Tokenizes a string separated by delimiter
 * @param str       The string to tokenize
 * @param delip     The delimeter e.g. ' ' 
 * @param ntoken    Number of tokens in the string (returns)
 * @return tokens   on success, NULL on failure
 *
 * Example:
 *  int n=0; 
 *  char **tokens=mutils_tokenize("this is"," ",&n);
 * 
 * The caller should free the tokens by calling:
 * mutils_free_tokens(tokens,ntokens)
 * Note: A token can be of MUTILS_MAX_TOKEN_LEN long
 */
char **mutilsTokenize(char *str,int delim,int *ntokens)
{
    char
        tbuf[MUTILS_MAX_TOKEN_LEN],
        **tokens=NULL;

    int
        j=0,
        count=0,
        allocated=0,
        i=0;

    char
        *p;

    *ntokens=0;
    if (str == NULL || *str == '\0')
        return(NULL);

    /* count how many token there first */
    for (p=str; *p; p++)
    {
        if (*p == delim)
        {
            count++;
        }
    }
    count++;
    /* allocate memory for tokens */
    tokens=(char **) malloc(count * sizeof(char *));
    MUTILS_CHECK_MALLOC(tokens);

    allocated=0;
    for (i=0; i < count; i++)
    {
        /*
        ** allocate memory for each token string, a token can 
        ** be of MUTILS_MAX_TOKEN_LEN characters long maximum
        */
        tokens[i]=(char *) malloc(MUTILS_MAX_TOKEN_LEN*sizeof(char));
        MUTILS_CHECK_MALLOC(tokens[i]);
        allocated++;
    }

    j=0;
    count=0;
    for (p=str; *p; p++)
    {
        if (*p != delim && *p != '\0')
        {
            if (j >= MUTILS_MAX_TOKEN_LEN)
            {
                (void) fprintf(stderr,"Buffer overflow detected\n");
                /* buffer overfow */
                goto ExitProcessing;
            }
            tbuf[j++]=*p;
        }
        else
        {
            /* we're in a new token */
            tbuf[j++]='\0';
            strcpy(tokens[count],tbuf);
            count++;
            j=0;
        }
    }
    if (j > 0 && j < MUTILS_MAX_TOKEN_LEN)
    {
        tbuf[j]='\0';
        mutilsStrncpy(tokens[count],tbuf,MUTILS_MAX_TOKEN_LEN-1);
    }

    count++;

    *ntokens=count;
    return(tokens);

ExitProcessing:
    mutilsFreeTokens(tokens,allocated);

    return(NULL);
}




#ifdef TEST

int main (int argc,char **argv) 
{
    int
        rc;

    rc=mutilsWhich(argv[1]);
    (void) fprintf(stderr,"rc=%d\n",rc);
    return(0);
}
#endif /* TEST */
