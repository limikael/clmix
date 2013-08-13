#ifndef __AUDIOSOURCE_TRIM_H__
#define __AUDIOSOURCE_TRIM_H__

#include "audiosource.h"

typedef struct AUDIOSOURCE_TRIM AUDIOSOURCE_TRIM;

AUDIOSOURCE_TRIM *audiosource_trim_create(AUDIOSOURCE *source, int trim, int length);

#endif
