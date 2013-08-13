#include <stdlib.h>

#include "audiosource.h"
#include "audiosource_vol.h"
#include "util.h"
#include "envelope.h"

struct AUDIOSOURCE_VOL {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	ENVELOPE *volume;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *t) {
	AUDIOSOURCE_VOL *this=(AUDIOSOURCE_VOL *)t;

	return audiosource_get_numsamples(this->source);
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *t, float *target, int numsamples) {
	AUDIOSOURCE_VOL *this=(AUDIOSOURCE_VOL *)t;

	//TRACE("vol get samples...\n");
	if (audiosource_get_samples(this->source, target, numsamples)) {
		int i;

		for (i=0; i<numsamples; i++) {
			float v=envelope_get_value(this->volume);
			//TRACE("v: %f\n",v);
			target[i*2]*=v;
			target[i*2+1]*=v;
		}

		return 1;
	}

	else {
		envelope_skip(this->volume,numsamples);
		return 0;
	}
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_VOL *this=(AUDIOSOURCE_VOL *)t;

	audiosource_set_streampos(this->source, pos);
	envelope_set_streampos(this->volume, pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_VOL *this=(AUDIOSOURCE_VOL *)s;

	free(this);
}

/**
 * Constructor.
 */
AUDIOSOURCE_VOL *audiosource_vol_create(AUDIOSOURCE *source, ENVELOPE *volume) {
	AUDIOSOURCE_VOL *this=nicemalloc(sizeof(AUDIOSOURCE_VOL));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;
	this->volume=volume;

	set_streampos((AUDIOSOURCE *)this,0);

	return this;
}
