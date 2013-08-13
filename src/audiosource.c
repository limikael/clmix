#include "audiosource.h"
#include "util.h"

/**
 * Get number of samples.
 */
int audiosource_get_numsamples(AUDIOSOURCE *source) {
	return source->get_numsamples_func(source);
}

/**
 * Zerofill silence if enabled.
 */
static int audiosource_zerofill_silence(AUDIOSOURCE *source, float *target, int numsamples) {
	if (!source->zerofill_silence)
		return 0;

	memset_float(target,0,numsamples*2);

	return 1;
}

/**
 * Get samples.
 */
int audiosource_get_samples(AUDIOSOURCE *source, float *target, int numsamples) {
	//TRACE("audiosource: reading at: %d, samples: %d\n",source->streampos,numsamples);

	if (source->streampos<-numsamples) {
		//TRACE("audiosource: reading before audio, returning all silence\n");
		source->streampos+=numsamples;
		return audiosource_zerofill_silence(source,target,numsamples);
	}

	if (source->streampos>audiosource_get_numsamples(source)) {
		//TRACE("audiosource: reading after audio, returning all silence\n");
		source->streampos+=numsamples;
		return audiosource_zerofill_silence(source,target,numsamples);
	}

	int usesamples=numsamples;

	if (source->streampos<0) {
		//TRACE("audiosource: reading overlapping chunk before\n");
		int frames=-source->streampos;

		memset_float(target,0,frames*2);
		target=&(target[frames*2]);
		usesamples-=frames;
		source->streampos=0;
	}

	int samples=usesamples;

	if (source->streampos+samples>source->get_numsamples_func(source)) {
		samples=source->get_numsamples_func(source)-source->streampos;
	}

	source->streampos+=usesamples;

	//TRACE("getting samples, source=%p, get saples func=%p\n",source,source->get_samples_func);
	if (!source->get_samples_func(source,target,samples)) {
		//TRACE("audiosource: actual source was silent\n");
		return audiosource_zerofill_silence(source,target,numsamples);
	}
	//TRACE("got samples\n");

	if (samples<usesamples) {
		//TRACE("audiosource: filling at the end: %d\n",usesamples-samples);
		memset_float(&target[samples*2],0,(usesamples-samples)*2);
	}

	return 1;
}

/**
 * Set stream position.
 */
void audiosource_set_streampos(AUDIOSOURCE *source, int streampos) {
	source->streampos=streampos;

	if (streampos<0)
		source->set_streampos_func(source,0);

	else if (streampos<source->get_numsamples_func(source))
		source->set_streampos_func(source,streampos);
}

/**
 * Dispose audio source.
 */
void audiosource_dispose(AUDIOSOURCE *source) {
	source->dispose_func(source);
}

/**
 * Initialize an audio source.
 * This is not intended to be used by the concrete audio source classes,
 * Not used directly.
 */
void audiosource_init(AUDIOSOURCE *source) {
	source->zerofill_silence=0;
	source->streampos=0;
}

/**
 * Should silence be filled with zeroes?
 */
void audiosource_enable_zerofill_silence(AUDIOSOURCE *source, int value) {
	source->zerofill_silence=value;
}

/**
 * Set memory to float value.
 */
void memset_float(float *target, float value, int count) {
	int i;

	for (i=0; i<count; i++)
		target[i]=value;
}

/**
 * Convert millisecs to samples.
 */
int millis_to_samples(int millis) {
	long long millis_l=(long long)millis;
	long long res_l;

	res_l=millis_l*441L/10L;

	return (int)res_l;
}

/**
 * Convert samples to millisecs.
 */
int samples_to_millis(int samples) {
	long long samples_l=(long long)samples;
	long long res_l;

	res_l=samples_l*10L/441L;

	return (int)res_l;
}
