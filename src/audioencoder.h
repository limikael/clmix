#ifndef __AUDIOENCODER_H__
#define __AUDIOENCODER_H__

#include <stdio.h>
#include "audiosource.h"

typedef struct AUDIOENCODER AUDIOENCODER;

AUDIOENCODER *audioencoder_create(char *target, AUDIOSOURCE *source, int position, char *format);
int audioencoder_encode(AUDIOENCODER *encoder, int length);
int audioencoder_eof(AUDIOENCODER *audioencoder);
void audioencoder_dispose(AUDIOENCODER *audioencoder);

#endif
