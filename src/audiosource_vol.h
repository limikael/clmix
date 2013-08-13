#ifndef __AUDIOSOURCE_VOL_H__
#define __AUDIOSOURCE_VOL_H__

#include "audiosource.h"
#include "envelope.h"

typedef struct AUDIOSOURCE_VOL AUDIOSOURCE_VOL;

AUDIOSOURCE_VOL *audiosource_vol_create(AUDIOSOURCE *source, ENVELOPE *volume);

#endif
