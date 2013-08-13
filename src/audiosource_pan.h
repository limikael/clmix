#ifndef __AUDIOSOURCE_PAN_H__
#define __AUDIOSOURCE_PAN_H__

#include "audiosource.h"
#include "envelope.h"

typedef struct AUDIOSOURCE_PAN AUDIOSOURCE_PAN;

AUDIOSOURCE_PAN *audiosource_pan_create(AUDIOSOURCE *source, ENVELOPE *pan);

#endif
