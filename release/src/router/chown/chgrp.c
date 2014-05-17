#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int main( int argc, char* argv[] ) {
   int i_arg_iter,
      gid,
      status = 0;
   struct group *grp;
   struct stat stbuf;

   /* Make sure we have enough to work with. */
   if( argc < 3 ) {
      printf( "usage: chgrp [gid] file\n" );
      exit( 4 );
   }

   if( isnumber( argv[1] ) ) {
      /* A numeric GID was specified. */
      gid = atoi(argv[1]);
   } else {
      /* Get the GID of the user from passwd. */
      if( (grp = getgrnam( argv[1] )) == NULL ) {
         printf( "unknown group id: %s\n", argv[1] );
         exit( 4 );
      }
      gid = grp->gr_gid;
   }

   /* Iterate through the remaining arguments, assuming they're filenames,    *
    * and change their owners.                                                */
   for( i_arg_iter=2; i_arg_iter < argc; i_arg_iter++ ) {
      stat( argv[i_arg_iter], &stbuf );
      if( chown( argv[i_arg_iter], stbuf.st_uid, gid ) < 0 ) {
         perror( argv[i_arg_iter] );
         status = 1;
      }
   }

   return status;
}

