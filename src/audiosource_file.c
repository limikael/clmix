#include <stdlib.h>
#include <stdio.h>

#include "audiosource.h"
#include "audiosource_file.h"
#include "util.h"
#include "ffsndin.h"

struct AUDIOSOURCE_FILE {
	AUDIOSOURCE source;

	FFSNDIN *ffsndin;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *s) {
	AUDIOSOURCE_FILE *source=(AUDIOSOURCE_FILE *)s;

	return ffsndin_get_num_frames(source->ffsndin);
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *s, float *target, int numsamples) {
	AUDIOSOURCE_FILE *source=(AUDIOSOURCE_FILE *)s;

	//TRACE("file get samples...\n");

	int samples=ffsndin_read(source->ffsndin,target,numsamples);

	if (samples!=numsamples)
		fail("audiosource_file: Unable to read from file.\n");

	//TRACE("got samples from file.\n");

	return 1;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *s, int pos) {
	AUDIOSOURCE_FILE *source=(AUDIOSOURCE_FILE *)s;

	ffsndin_seek(source->ffsndin,pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_FILE *source=(AUDIOSOURCE_FILE *)s;

	ffsndin_close(source->ffsndin);
	free(source);
}

/**
 * Constructor.
 */
AUDIOSOURCE_FILE *audiosource_file_create(char *fn) {
	AUDIOSOURCE_FILE *source=nicemalloc(sizeof(AUDIOSOURCE_FILE));

	source->ffsndin=ffsndin_open(fn); //,SFM_READ,&source->sf_info);

	if (!source->ffsndin)
		fail("Unable to open input file: %s",fn);

/*	if (source->sf_info.channels!=2)
		fail("Only 2 channel stereo files supported.");*/

	source->source.get_numsamples_func=get_numsamples;
	source->source.get_samples_func=get_samples;
	source->source.set_streampos_func=set_streampos;
	source->source.dispose_func=dispose;

	return source;
}
