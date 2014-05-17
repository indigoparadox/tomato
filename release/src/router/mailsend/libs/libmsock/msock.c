/* a quick port of my libmsock routine to MS NT */
/* muquit@Aug-20-2005  Mar 01  Eastern Standard Time 2001 */

#include "msock.h"

struct in_addr *atoAddr(char *address)
{
    struct hostent
        *host;

    static struct in_addr
        saddr;

    saddr.s_addr=inet_addr(address);
    if (saddr.s_addr != -1)
        return (&saddr);

    host=gethostbyname(address);
    if (host != (struct hostent *) NULL)
        return ((struct in_addr *) *host->h_addr_list);

    return ((struct in_addr *) NULL);
}

#ifdef WINNT
/* returns 0 on success -1 on failure */
int initWinSock(void)
{
    WORD
        version_requested;

    WSADATA
        wsa_data;

    int
        err;

    version_requested=MAKEWORD(2,0);
    err=WSAStartup(version_requested,&wsa_data);
    if (err != 0)
    {
        (void) fprintf(stderr," Unable to initialize winsock (%d)\n",err);
        return(-1);
    }

    return(0);
}
#endif /* WINNT */

/* returns SOCKET on success INVALID_SOCKET on failure */
SOCKET clientSocket(char *address,int port)
{
    SOCKET
        s;

    struct sockaddr_in
        sa;

    struct in_addr
        *addr;

    int
        rc;

#ifdef WINNT
    rc=initWinSock();
    if (rc != 0)
        return(INVALID_SOCKET);
#endif /* WINNT */
    
    addr=atoAddr(address);
    if (addr == NULL)
    {
        (void) fprintf(stderr," Invalid address: %s\n",address);
        return(INVALID_SOCKET);
    }

    memset((char *) &sa,0,sizeof(sa));
    sa.sin_family=AF_INET;
    sa.sin_port=htons(port);
    sa.sin_addr.s_addr=addr->s_addr;

    /* open the socket */
    s=socket(AF_INET,SOCK_STREAM,PF_UNSPEC);
    if (s == INVALID_SOCKET)
    {
        (void) fprintf(stderr," Could not create socket\n");
        return(INVALID_SOCKET);
    }

    /* connect */
    connect(s,(struct sockaddr *) &sa,sizeof(sa));


    return(s);
}



/*
** this function writes a character string out to a socket.
** it returns -1 if the connection is closed while it is trying to
** write
*/
static int sockWrite(SOCKET sock,char *str,size_t count)
{
    size_t
        bytesSent=0;

    int
        thisWrite;

    while (bytesSent < count)
    {
       thisWrite=send(sock,str,count-bytesSent,0);
       /*
       (void) fprintf(stderr,"str=%s\n",str);
       (void) fprintf(stderr,"count=%d\n",count);
       */

       if (thisWrite <= 0)
          return (thisWrite);

        bytesSent += thisWrite;
        str += thisWrite;
    }
    return (count);
}

int sockPuts(SOCKET sock,char *str)
{
    return (sockWrite(sock,str,strlen(str)));
}

int sockGets(SOCKET sockfd,char *str,size_t count)
{
    int
        bytesRead;

    int
        totalCount=0;

    char
        buf[1],
        *currentPosition;

    char
        lastRead=0;

    currentPosition=str;

    while (lastRead != 10)
    {
        bytesRead=recv(sockfd,buf,1,0);
        if (bytesRead <= 0)
        {
            /*
            ** the other side may have closed unexpectedly
            */
            return (-1);
        }
        lastRead=buf[0];

        if ((totalCount < count) && (lastRead != 10)
            && (lastRead != 13))
        {
            *currentPosition=lastRead;
            currentPosition++;
            totalCount++;
        }
    }
    if (count > 0)
        *currentPosition=0;

    return (totalCount);
}
