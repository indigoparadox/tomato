
#include "util.h"

int isnumber( char* s ) {
   char c;

   while( (c = *s++) ) {
      if( !isdigit( c ) ) {
         return(0);
      }
   }

   return 1;
}

