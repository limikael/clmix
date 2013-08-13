#ifndef __ENVELOPE_H__
#define __ENVELOPE_H__

typedef struct ENVELOPE ENVELOPE;

struct ENVELOPE {
	float (*get_value_func)(ENVELOPE *envelope, int pos);

	int streampos;
	int used_values;
	int frame_size;
	float current;
};

void envelope_init(ENVELOPE *envelope);
float envelope_get_value(ENVELOPE *envelope);
void envelope_set_streampos(ENVELOPE *envelope, int pos);
void envelope_skip(ENVELOPE *envelope, int skip);

#endif
