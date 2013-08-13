#include <stdlib.h>

#include "audiosource.h"
#include "audiosource_pan.h"
#include "util.h"
#include "envelope.h"

struct AUDIOSOURCE_PAN {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	ENVELOPE *pan;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *t) {
	AUDIOSOURCE_PAN *this=(AUDIOSOURCE_PAN *)t;

	return audiosource_get_numsamples(this->source);
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *t, float *target, int numsamples) {
	AUDIOSOURCE_PAN *this=(AUDIOSOURCE_PAN *)t;

	//TRACE("pan get samples...\n");
	if (audiosource_get_samples(this->source, target, numsamples)) {
		int i;

		for (i=0; i<numsamples; i++) {
			float leftIn=target[i*2];
			float rightIn=target[i*2+1];
			float leftOut;
			float rightOut;
			float pan=envelope_get_value(this->pan);

			if (pan>0) {
				leftOut=leftIn*(1-pan);
				rightOut=rightIn+leftIn*pan;
			}

			else if (pan<0) {
				pan=-pan;
				leftOut=leftIn+rightIn*pan;
				rightOut=rightIn*(1-pan);
			}

			else {
				leftOut=leftIn;
				rightOut=rightIn;
			}
			
			target[i*2]=leftOut;
			target[i*2+1]=rightOut;
		}

		return 1;
	}

	else
		return 0;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_PAN *this=(AUDIOSOURCE_PAN *)t;

	audiosource_set_streampos(this->source, pos);
	envelope_set_streampos(this->pan, pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_PAN *this=(AUDIOSOURCE_PAN *)s;

	free(this);
}

/**
 * Constructor.
 */
AUDIOSOURCE_PAN *audiosource_pan_create(AUDIOSOURCE *source, ENVELOPE *pan) {
	AUDIOSOURCE_PAN *this=nicemalloc(sizeof(AUDIOSOURCE_PAN));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;
	this->pan=pan;

	set_streampos((AUDIOSOURCE *)this,0);

	return this;
}
