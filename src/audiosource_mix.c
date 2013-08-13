#include <stdlib.h>

#include "audiosource.h"
#include "audiosource_mix.h"
#include "util.h"

struct AUDIOSOURCE_MIX {
	AUDIOSOURCE this;
	AUDIOSOURCE **sources;
	int numsources;
	float *tmpbuf;
	int tmpbufsamples;
};

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *t) {
	AUDIOSOURCE_MIX *this=(AUDIOSOURCE_MIX *)t;

	int len=0;
	int i;

	for (i=0; i<this->numsources; i++) {
		int l=audiosource_get_numsamples(this->sources[i]);

		if (l>len)
			len=l;
	}

	return len;
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *t, float *target, int numsamples) {
	AUDIOSOURCE_MIX *this=(AUDIOSOURCE_MIX *)t;

	if (numsamples>this->tmpbufsamples) {
		if (this->tmpbuf)
			free(this->tmpbuf);

		this->tmpbuf=nicemalloc(numsamples*2*sizeof (float));
		this->tmpbufsamples=numsamples;
	}

	//TRACE("mix get samples, sources: %d, numsamples: %d\n",this->numsources, numsamples);

	int i;
	int havetarget=0;

	//TRACE("allocating float: %d\n",numsamples*2);
//	float tmp[numsamples*2];
	//TRACE("will loop\n");

	for (i=0; i<this->numsources; i++) {
		//TRACE("loop iter: %d\n",i);
		if (!havetarget) {
			havetarget=audiosource_get_samples(this->sources[i],target,numsamples);
		}

		else {
			int got=audiosource_get_samples(this->sources[i],this->tmpbuf,numsamples);

			if (got) {
				int n;

				for (n=0; n<numsamples*2; n++)
					target[n]+=this->tmpbuf[n];
			}
		}
	}

	return havetarget;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_MIX *this=(AUDIOSOURCE_MIX *)t;
	int i;

	for (i=0; i<this->numsources; i++)
		audiosource_set_streampos(this->sources[i], pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_MIX *this=(AUDIOSOURCE_MIX *)s;

	free(this);
}

/**
 * Constructor.
 */
AUDIOSOURCE_MIX *audiosource_mix_create(AUDIOSOURCE **sources, int numsources) {
	AUDIOSOURCE_MIX *this=nicemalloc(sizeof(AUDIOSOURCE_MIX));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;
	this->tmpbuf=NULL;
	this->tmpbufsamples=0;

	this->sources=nicemalloc(numsources*sizeof(AUDIOSOURCE *));

	int i;
	for (i=0; i<numsources; i++)
		this->sources[i]=sources[i];

	this->numsources=numsources;

	return this;
}
