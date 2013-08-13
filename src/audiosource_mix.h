#ifndef __AUDIOSOURCE_MIX_H__
#define __AUDIOSOURCE_MIX_H__

#include "audiosource.h"

typedef struct AUDIOSOURCE_MIX AUDIOSOURCE_MIX;

AUDIOSOURCE_MIX *audiosource_mix_create(AUDIOSOURCE **sources, int numsources);

#endif
