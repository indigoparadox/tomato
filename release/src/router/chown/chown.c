#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int main( int argc, char* argv[] ) {
   int i_arg_iter,
      uid,
      status = 0;
   struct passwd *pwd;
   struct stat stbuf;

   /* Make sure we have enough to work with. */
   if( argc < 3 ) {
      printf( "usage: chown [uid] file\n" );
      exit( 4 );
   }

   if( isnumber( argv[1] ) ) {
      /* A numeric UID was specified. */
      uid = atoi(argv[1]);
   } else {
      /* Get the UID of the user from passwd. */
      if( (pwd = getpwnam( argv[1] )) == NULL ) {
         printf( "unknown user id: %s\n", argv[1] );
         exit( 4 );
      }
      uid = pwd->pw_uid;
   }

   /* Iterate through the remaining arguments, assuming they're filenames,    *
    * and change their owners.                                                */
   for( i_arg_iter=2; i_arg_iter < argc; i_arg_iter++ ) {
      stat( argv[i_arg_iter], &stbuf );
      if( chown( argv[i_arg_iter], uid, stbuf.st_gid ) < 0 ) {
         perror( argv[i_arg_iter] );
         status = 1;
      }
   }

   return status;
}

