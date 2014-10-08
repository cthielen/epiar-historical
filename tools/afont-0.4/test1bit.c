/* test1bit.c */

#include "afont_gl.h"

int main( int argc, char **argv )
{
  afont *a;

  if(argc != 2)
    return 0;

  a = afont_load(argv[1]);

  afont_dump_char(a, 'h');
  afont_gl_make_bitmaps(a);

  afont_free(a);
  return 0;
}
