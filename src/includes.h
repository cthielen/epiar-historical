#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifdef LINUX
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <libintl.h>
#include <netinet/in.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include "SDL.h"
#include "SDL_image.h"
