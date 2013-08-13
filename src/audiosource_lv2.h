#ifndef __AUDIOSOURCE_LV2_H__
#define __AUDIOSOURCE_LV2_H__

#include "audiosource.h"

typedef struct AUDIOSOURCE_LV2 AUDIOSOURCE_LV2;

AUDIOSOURCE_LV2 *audiosource_lv2_create(AUDIOSOURCE *source, char *uri, char **inputs, char **outputs);
void audiosource_lv2_set_control(AUDIOSOURCE_LV2 *this, char *control, float value);

#endif
