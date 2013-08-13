#include "envelope.h"
#include "util.h"

/**
 * Init envelope base structure.
 */
void envelope_init(ENVELOPE *envelope) {
	envelope->streampos=0;
	envelope->used_values=-1;
	envelope->frame_size=882;
	envelope->current=0;
}

/**
 * Get value at current position.
 */
float envelope_get_value(ENVELOPE *envelope) {
	envelope->streampos++;

	if (envelope->used_values>=0 && envelope->used_values<envelope->frame_size) {
		//TRACE("using current\n");
		envelope->used_values++;
		return envelope->current;
	}

	//TRACE("'*************************caling\n");
	envelope->used_values=0;
	envelope->current=envelope->get_value_func(envelope,envelope->streampos);
	//TRACE("get value sp: %d, value: %f\n",envelope->streampos,envelope->current);
	return envelope->current;
}

/**
 * Set streampos.
 */
void envelope_set_streampos(ENVELOPE *envelope, int pos) {
	envelope->used_values=-1;
	envelope->streampos=pos;
}

/**
 * Skip.
 */
void envelope_skip(ENVELOPE *envelope, int skip) {
	envelope->used_values=-1;
	envelope->streampos+=skip;

	//TRACE("skipping, frames=%d, now=%d\n",skip,envelope->streampos);
}
