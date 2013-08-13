#ifndef __AUDIOSOURCE_FILE_H__
#define __AUDIOSOURCE_FILE_H__

#include "audiosource.h"

typedef struct AUDIOSOURCE_FILE AUDIOSOURCE_FILE;

AUDIOSOURCE_FILE *audiosource_file_create(char *fn);

#endif
