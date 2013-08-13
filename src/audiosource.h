#ifndef __AUDIOSOURCE_H__
#define __AUDIOSOURCE_H__

typedef struct AUDIOSOURCE AUDIOSOURCE;

struct AUDIOSOURCE {
	int (*get_numsamples_func)(AUDIOSOURCE *source);
	int (*get_samples_func)(AUDIOSOURCE *source, float *target, int num_samples);
	void (*set_streampos_func)(AUDIOSOURCE *source, int stream_pos);
	void (*dispose_func)(AUDIOSOURCE *source);

	int streampos;
	int zerofill_silence;
};

int audiosource_get_numsamples(AUDIOSOURCE *source);
int audiosource_get_samples(AUDIOSOURCE *source, float *target, int numsamples);
void audiosource_set_streampos(AUDIOSOURCE *source, int streampos);
void audiosource_dispose(AUDIOSOURCE *source);
void audiosource_init(AUDIOSOURCE *source);
void audiosource_enable_zerofill_silence(AUDIOSOURCE *source, int value);

void memset_float(float *target, float value, int count);
int millis_to_samples(int millis);
int samples_to_millis(int samples);

#endif
