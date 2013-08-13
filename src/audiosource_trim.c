#include <stdlib.h>

#include "audiosource.h"
#include "audiosource_trim.h"
#include "util.h"

struct AUDIOSOURCE_TRIM {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	int trim_samples;
	int length_samples;

	int pos;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *t) {
	AUDIOSOURCE_TRIM *this=(AUDIOSOURCE_TRIM *)t;

	return this->length_samples;
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *t, float *target, int numsamples) {
	AUDIOSOURCE_TRIM *this=(AUDIOSOURCE_TRIM *)t;

	if (this->pos<-numsamples) {
		audiosource_get_samples(this->source,target,numsamples);
		this->pos+=numsamples;
		return 0;
	}

	if (this->pos>this->length_samples) {
		audiosource_get_samples(this->source,target,numsamples);
		this->pos+=numsamples;
		return 0;
	}

	int res=audiosource_get_samples(this->source,target,numsamples);

	if (res) {
		if (this->pos<0)
			memset_float(target,0,-this->pos*2);

		if (this->pos+numsamples>this->length_samples) {
			int to=this->pos+numsamples;
			int fill=to-this->length_samples;

			memset_float(&target[2*(numsamples-fill)],0,2*fill);
		}
	}

	this->pos+=numsamples;
	return res;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_TRIM *this=(AUDIOSOURCE_TRIM *)t;

	this->pos=pos;
	audiosource_set_streampos(this->source, pos+this->trim_samples);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_TRIM *this=(AUDIOSOURCE_TRIM *)s;

	free(this);
}

/**
 * Constructor.
 */
AUDIOSOURCE_TRIM *audiosource_trim_create(AUDIOSOURCE *source, int trim, int length) {
	AUDIOSOURCE_TRIM *this=nicemalloc(sizeof(AUDIOSOURCE_TRIM));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;
	this->trim_samples=millis_to_samples(trim);
	this->length_samples=millis_to_samples(length);

	set_streampos((AUDIOSOURCE *)this,0);

	return this;
}
