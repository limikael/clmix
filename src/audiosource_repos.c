#include <stdlib.h>

#include "audiosource.h"
#include "audiosource_repos.h"
#include "util.h"

struct AUDIOSOURCE_REPOS {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	long pos;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *t) {
	AUDIOSOURCE_REPOS *this=(AUDIOSOURCE_REPOS *)t;

	int numsamples=this->pos+audiosource_get_numsamples(this->source);

	//TRACE("repos numsamples: %d secs: %d\n",numsamples,numsamples/44100);
	return numsamples;
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *t, float *target, int numsamples) {
	AUDIOSOURCE_REPOS *this=(AUDIOSOURCE_REPOS *)t;

	return audiosource_get_samples(this->source, target, numsamples);
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_REPOS *this=(AUDIOSOURCE_REPOS *)t;

	audiosource_set_streampos(this->source, pos-this->pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_REPOS *this=(AUDIOSOURCE_REPOS *)s;

	free(this);
}

/**
 * Constructor.
 */
AUDIOSOURCE_REPOS *audiosource_repos_create(AUDIOSOURCE *source, int pos) {
	AUDIOSOURCE_REPOS *this=nicemalloc(sizeof(AUDIOSOURCE_REPOS));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;
	this->pos=(long long)pos*44100L/1000L;

	set_streampos((AUDIOSOURCE *)this,0);

	return this;
}
