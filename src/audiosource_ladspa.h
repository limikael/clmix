#ifndef __AUDIOSOURCE_LADSPA_H__
#define __AUDIOSOURCE_LADSPA_H__

#include "audiosource.h"
#include "envelope.h"

typedef struct AUDIOSOURCE_LADSPA AUDIOSOURCE_LADSPA;

AUDIOSOURCE_LADSPA *audiosource_ladspa_create(AUDIOSOURCE *source, char *lib_file_name, char *plugin_name);
void audiosource_ladspa_set_control(AUDIOSOURCE_LADSPA *this, char *control, ENVELOPE *value);
void audiosource_ladspa_set_extend_time(AUDIOSOURCE_LADSPA *this, int millisecs);

#endif
