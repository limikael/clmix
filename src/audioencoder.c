#include <stdio.h>
#include <stdlib.h>
#include <lame/lame.h>
#include "ffsndout.h"
#include "util.h"
#include "audiosource.h"
#include "audioencoder.h"

struct AUDIOENCODER {
	FFSNDOUT *ffsndout;
	AUDIOSOURCE *source;
	long framepos;
};

/**
 * Encode.
 */
int audioencoder_encode(AUDIOENCODER *encoder, int length) {
	if (audioencoder_eof(encoder))
		return 0;

	int numframes=length*44100/1000;

	if (encoder->framepos+numframes>audiosource_get_numsamples(encoder->source))
		numframes=audiosource_get_numsamples(encoder->source)-encoder->framepos;

	float samples[2*numframes*sizeof(float)];

	TRACE("geting samples: %d\n",numframes);
	int res=audiosource_get_samples(encoder->source,samples,numframes);
	TRACE("get samples res: %d\n",res);
	if (!res)
		memset_float(samples,0,numframes*2);

	int i;
	for (i=0; i<2*numframes; i++) {
		if (samples[i]>1.0)
			samples[i]=1.0;

		if (samples[i]<-1.0)
			samples[i]=-1.0;
	}

	size_t written=ffsndout_write(encoder->ffsndout,samples,numframes);

	if (written!=numframes)
		fail("unable to write to wav file.");

	encoder->framepos+=numframes;
	return numframes;
}

/**
 * Eof?
 */
int audioencoder_eof(AUDIOENCODER *encoder) {
	if (encoder->framepos>=audiosource_get_numsamples(encoder->source))
		return 1;

	else
		return 0;
}

/**
 * Dispose.
 */
void audioencoder_dispose(AUDIOENCODER *encoder) {
	TRACE("**** disposing encoder..\n");

	ffsndout_close(encoder->ffsndout);

	free(encoder);
}

/**
 * Create.
 */
AUDIOENCODER *audioencoder_create(char *target, AUDIOSOURCE *source, int position, char *format) {
	AUDIOENCODER *encoder;

	encoder=nicemalloc(sizeof(AUDIOENCODER));

	encoder->framepos=(long long)position*44100/1000;
	encoder->ffsndout=ffsndout_open(target,format);

	if (!encoder->ffsndout)
		fail("Unable to open outfile.");

	encoder->source=source;
	audiosource_set_streampos(encoder->source,encoder->framepos);

	return encoder;	
}
