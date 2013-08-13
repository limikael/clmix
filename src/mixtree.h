#ifndef __MIXTREE_H__
#define __MIXTREE_H__

#include <stdio.h>

#include "mixtree.h"
#include "audiosource.h"

AUDIOSOURCE *mixtree_loadf(FILE *f);
AUDIOSOURCE *mixtree_load_file(char *filename);

#endif
