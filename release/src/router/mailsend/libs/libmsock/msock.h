#ifndef MSOCK_H
#define MSOCK_H

#ifdef WINNT
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <windows.h>

#else

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  /* ULTRIX didn't like stat with types*/
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>  /* for inet_ntoa */
#include <time.h>  /* for ctime */
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <limits.h>

#undef SOCKET
#define SOCKET int

#undef INVALID_SOCKET
#define INVALID_SOCKET -1
#define _fileno fileno
#define _isatty isatty

#endif /* ! WINNT */

SOCKET clientSocket(char *,int);
int    sockGets(SOCKET,char *,size_t);
int sockPuts(SOCKET sock,char *str);

#endif /* ! MSOC_H */
