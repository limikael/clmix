#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "audiosource.h"
#include "audiosource_ladspa.h"
#include "util.h"
#include "ladspa_utils.h"
#include "envelope.h"

struct AUDIOSOURCE_LADSPA {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	LADSPA_Data *control_values;
	int *have_controls;
	void *ladspa_library;
	const LADSPA_Descriptor *ladspa_descriptor;
	LADSPA_Handle instance;

	float *leftinbuf, *rightinbuf;
	float *leftoutbuf, *rightoutbuf;
	int bufsize;
	int controls_checked;
	int extend_samples;
	int samples_extended;
	int extension_state;
	ENVELOPE **envelopes;

	int inputs[2],outputs[2];
};

/**
 * Check buf size.
 */
static void check_buf_size(AUDIOSOURCE_LADSPA *this, int size) {
	if (size<=this->bufsize)
		return;

	TRACE("** ALLOCATING BUFFERS\n");
	this->bufsize=size;

	this->leftinbuf=nicerealloc(this->leftinbuf,this->bufsize*sizeof (LADSPA_Data));
	this->rightinbuf=nicerealloc(this->rightinbuf,this->bufsize*sizeof (LADSPA_Data));
	this->leftoutbuf=nicerealloc(this->leftoutbuf,this->bufsize*sizeof (LADSPA_Data));
	this->rightoutbuf=nicerealloc(this->rightoutbuf,this->bufsize*sizeof (LADSPA_Data));
}

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *s) {
	AUDIOSOURCE_LADSPA *this=(AUDIOSOURCE_LADSPA *)s;

	return audiosource_get_numsamples(this->source)+this->extend_samples;
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *s, float *target, int numsamples) {
	AUDIOSOURCE_LADSPA *this=(AUDIOSOURCE_LADSPA *)s;

	//TRACE("ladspa get samples\n");

	if (!this->controls_checked) {
		int i;

		for (i=0; i<this->ladspa_descriptor->PortCount; i++)
			if (!this->have_controls[i])
				fail("Control '%s' for plugin %s is not provided and has no default value.",
					this->ladspa_descriptor->PortNames[i],this->ladspa_descriptor->Label);

		this->controls_checked=1;
	}

	if (audiosource_get_samples(this->source,target,numsamples)) {
		this->extension_state=1;
		this->samples_extended=0;
	}

	else {
		int i;

		if (!this->extension_state || !this->extend_samples) {
			for (i=0; i<this->ladspa_descriptor->PortCount; i++) {
				if (this->envelopes[i])
					envelope_skip(this->envelopes[i],numsamples);
			}

			return 0;
		}

		memset_float(target,0,numsamples*2);
		this->samples_extended+=numsamples;
		if (this->samples_extended>=this->extend_samples)
			this->extension_state=0;
	}

	check_buf_size(this, numsamples);

	this->ladspa_descriptor->connect_port(this->instance,this->inputs[0],this->leftinbuf);
	this->ladspa_descriptor->connect_port(this->instance,this->inputs[1],this->rightinbuf);
	this->ladspa_descriptor->connect_port(this->instance,this->outputs[0],this->leftoutbuf);
	this->ladspa_descriptor->connect_port(this->instance,this->outputs[1],this->rightoutbuf);

	int i;
	int processed=0;
	int fsize=1024;

	while (processed<numsamples) {
		int todo;

		if (processed+fsize>=numsamples)
			todo=numsamples-processed;

		else
			todo=fsize;

		for (i=0; i<todo; i++) {
			this->leftinbuf[i]=target[(processed+i)*2];
			this->rightinbuf[i]=target[(processed+i)*2+1];
		}

		for (i=0; i<this->ladspa_descriptor->PortCount; i++) {
			if (this->envelopes[i]) {
				this->control_values[i]=envelope_get_value(this->envelopes[i]);
				envelope_skip(this->envelopes[i],fsize-1);
			}
		}

		this->ladspa_descriptor->run(this->instance,todo);

		for (i=0; i<todo; i++) {
			target[(processed+i)*2]=this->leftoutbuf[i];
			target[(processed+i)*2+1]=this->rightoutbuf[i];
		}

		processed+=todo;
	}

	return 1;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *s, int pos) {
	AUDIOSOURCE_LADSPA *this=(AUDIOSOURCE_LADSPA *)s;
	int i;

	TRACE("setting pos for source\n");
	audiosource_set_streampos(this->source,pos);
	this->extension_state=0;

	TRACE("setting envelope pos\n");

	for (i=0; i<this->ladspa_descriptor->PortCount; i++)
		if (this->envelopes[i])
			envelope_set_streampos(this->envelopes[i],pos);

	TRACE("done setting env pos\n");
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
	AUDIOSOURCE_LADSPA *this=(AUDIOSOURCE_LADSPA *)s;

	audiosource_dispose(this->source);
	free(this);
}

/**
 * Set control.
 */
void audiosource_ladspa_set_control(AUDIOSOURCE_LADSPA *this, char *control, ENVELOPE *value) {
	int i;

	for (i=0; i<this->ladspa_descriptor->PortCount; i++) {
		if (!strcmp(this->ladspa_descriptor->PortNames[i],control)) {
			envelope_set_streampos(value,this->this.streampos);
			this->have_controls[i]=1;
			this->envelopes[i]=value;
			return;
		}
	}

	fail("Unknown port '%s' for plugin %s",control,this->ladspa_descriptor->Label);
}

/**
 * Set how many millisecs the underlying source should be extended.
 */
void audiosource_ladspa_set_extend_time(AUDIOSOURCE_LADSPA *this, int millisecs) {
	this->extend_samples=millis_to_samples(millisecs);
}

/**
 * Constructor.
 */
AUDIOSOURCE_LADSPA *audiosource_ladspa_create(AUDIOSOURCE *source, char *lib_file_name, char *plugin_name) {
	TRACE("creating ladspa source..\n");
	AUDIOSOURCE_LADSPA *this=nicemalloc(sizeof(AUDIOSOURCE_LADSPA));

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;

	this->ladspa_library=loadLADSPAPluginLibrary(lib_file_name);
	this->ladspa_descriptor=findLADSPAPluginDescriptor(this->ladspa_library,lib_file_name,plugin_name);

	this->control_values=nicemalloc(sizeof (LADSPA_Data)*this->ladspa_descriptor->PortCount);
	this->have_controls=nicemalloc(sizeof (int)*this->ladspa_descriptor->PortCount);
	this->envelopes=nicemalloc(sizeof (ENVELOPE *)*this->ladspa_descriptor->PortCount);

	this->instance=this->ladspa_descriptor->instantiate(this->ladspa_descriptor,44100);

	if (!this->instance)
		fail("Unable to instantiate %s",plugin_name);

	int i;
	int audioinputs=0, audiooutputs=0;

	for (i=0; i<this->ladspa_descriptor->PortCount; i++)
		this->have_controls[i]=0;

	for (i=0; i<this->ladspa_descriptor->PortCount; i++) {
		this->envelopes[i]=NULL;

		TRACE("port %i: %s\n",i,this->ladspa_descriptor->PortNames[i]);

		LADSPA_PortDescriptor d=this->ladspa_descriptor->PortDescriptors[i];
		if (LADSPA_IS_PORT_AUDIO(d)) {
			if (LADSPA_IS_PORT_INPUT(d)) {
				this->inputs[audioinputs++]=i;
				if (audioinputs>2)
					fail("Expected exactly 2 audio inputs.");
			}

			if (LADSPA_IS_PORT_OUTPUT(d)) {
				this->outputs[audiooutputs++]=i;
				if (audiooutputs>2)
					fail("Expected exactly 2 audio outputs.");
			}
		}

		if (LADSPA_IS_PORT_CONTROL(d))
			this->ladspa_descriptor->connect_port(this->instance,i,&this->control_values[i]);

		if (LADSPA_IS_PORT_CONTROL(d) && LADSPA_IS_PORT_INPUT(d)) {
			const LADSPA_PortRangeHint *range_hint=&(this->ladspa_descriptor->PortRangeHints[i]);
			LADSPA_Data def;
			int have_hint;
			have_hint=getLADSPADefault(range_hint,44100,&def);

			if (have_hint==0) {
				this->control_values[i]=def;
				this->have_controls[i]=1;
			}
		}

		else
			this->have_controls[i]=1;
	}

	if (audioinputs!=2 || audiooutputs!=2)
		fail("Works only with exactly 2 inputs and 2 outputs.");

	this->leftinbuf=0;
	this->rightinbuf=0;
	this->leftoutbuf=0;
	this->rightoutbuf=0;
	this->bufsize=0;
	this->controls_checked=0;
	this->extend_samples=44100;
	this->extension_state=0;
	this->samples_extended=0;

	if (this->ladspa_descriptor->activate)
		this->ladspa_descriptor->activate(this->instance);

	TRACE("almost done creating ladspa source, setting streampos\n");
	set_streampos((AUDIOSOURCE *)this,0);
	TRACE("ladspa source created\n");

	return this;
}
