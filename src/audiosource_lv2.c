#include <stdlib.h>
#include <stdio.h>
#include <sndfile.h>
#include <slv2/slv2.h>
#include <string.h>

#include "audiosource.h"
#include "audiosource_lv2.h"
#include "util.h"

struct AUDIOSOURCE_LV2 {
	AUDIOSOURCE this;
	AUDIOSOURCE *source;

	SLV2Plugin plugin;
	SLV2Instance instance;

	int leftin, rightin;
	int leftout, rightout;

	float *leftinbuf, *rightinbuf;
	float *leftoutbuf, *rightoutbuf;
	int bufsize;

	char **portnames;
	float *controlvalues;
};

static SLV2World world=NULL;
static SLV2Plugins plugins=NULL;

/**
 * Get number of samples.
 */
static int get_numsamples(AUDIOSOURCE *s) {
	AUDIOSOURCE_LV2 *this=(AUDIOSOURCE_LV2 *)s;

	return audiosource_get_numsamples(this->source);
}

/**
 * Check buf size.
 */
static void check_buf_size(AUDIOSOURCE_LV2 *this, int size) {
	if (size<this->bufsize)
		return;

	TRACE("** ALLOCATING BUFFERS\n");
	this->bufsize=size;

	this->leftinbuf=nicerealloc(this->leftinbuf,this->bufsize*sizeof (float));
	this->rightinbuf=nicerealloc(this->rightinbuf,this->bufsize*sizeof (float));
	this->leftoutbuf=nicerealloc(this->leftoutbuf,this->bufsize*sizeof (float));
	this->rightoutbuf=nicerealloc(this->rightoutbuf,this->bufsize*sizeof (float));
}

/**
 * Get samples.
 */
static int get_samples(AUDIOSOURCE *s, float *target, int numsamples) {
	AUDIOSOURCE_LV2 *this=(AUDIOSOURCE_LV2 *)s;
	int i;

	if (!audiosource_get_samples(this->source,target,numsamples))
		return 0;

	check_buf_size(this, numsamples);

	for (i=0; i<numsamples; i++) {
		this->leftinbuf[i]=target[i*2];
		this->rightinbuf[i]=target[i*2+1];
	}

	slv2_instance_connect_port(this->instance,this->leftin,this->leftinbuf);
	slv2_instance_connect_port(this->instance,this->rightin,this->rightinbuf);
	slv2_instance_connect_port(this->instance,this->leftout,this->leftoutbuf);
	slv2_instance_connect_port(this->instance,this->rightout,this->rightoutbuf);

	TRACE("running lv2\n");
//	slv2_instance_run(this->instance, numsamples);
	slv2_instance_run(this->instance, 1);
	TRACE("done running\n");

	for (i=0; i<numsamples; i++) {
		target[i*2]=this->leftoutbuf[i];
		target[i*2+1]=this->rightoutbuf[i];
	}

	return numsamples;
}

/**
 * Set streampos.
 */
static void set_streampos(AUDIOSOURCE *t, int pos) {
	AUDIOSOURCE_LV2 *this=(AUDIOSOURCE_LV2 *)t;

	audiosource_set_streampos(this->source, pos);
}

/**
 * Destructor.
 */
static void dispose(AUDIOSOURCE *s) {
}

/**
 * Set numeric control value.
 */
void audiosource_lv2_set_control(AUDIOSOURCE_LV2 *this, char *control, float value) {
}

/**
 * Get port number.
 */
static int get_port_number_by_name(AUDIOSOURCE_LV2 *this, char *name) {
	int i;

	for (i=0; i<slv2_plugin_get_num_ports(this->plugin); i++) {
		if (!strcmp(this->portnames[i],name))
			return i;
	}

	fail("Unknown port name: %s\n",name);
	return -1;
}

/**
 * Constructor.
 */
AUDIOSOURCE_LV2 *audiosource_lv2_create(AUDIOSOURCE *source, char *uri, char **inputs, char **outputs) {
	AUDIOSOURCE_LV2 *this=nicemalloc(sizeof(AUDIOSOURCE_LV2));

	if (!world) {
		TRACE("Initializing LV2.\n");
		world=slv2_world_new();
		if (!world)
			fail("Unable to initialize lv2 world.");

		slv2_world_load_all(world);
		TRACE("LV2 plugins loaded.\n");

		plugins = slv2_world_get_all_plugins(world);
	}

	SLV2Value uri_value = slv2_value_new_uri(world,uri);
	this->plugin=slv2_plugins_get_by_uri(plugins,uri_value);
	slv2_value_free(uri_value);

	if (!this->plugin)
		fail("Unable to get plugin: %s",uri);

	this->instance=slv2_plugin_instantiate(this->plugin, 44100, NULL);

	if (!this->instance)
		fail("Unable to instantiate plugin.");

	this->portnames=nicemalloc(sizeof (char *)*slv2_plugin_get_num_ports(this->plugin));
	this->controlvalues=nicemalloc(sizeof (float)*slv2_plugin_get_num_ports(this->plugin));

	TRACE("number of ports: %d\n",slv2_plugin_get_num_ports(this->plugin));
	int i;

	SLV2Value      input_class;   /**< Input port class (URI) */
	SLV2Value      control_class; /**< Control port class (URI) */
	SLV2Value      output_class;   /**< Audio port class (URI) */

	input_class = slv2_value_new_uri(world, SLV2_PORT_CLASS_INPUT);
	output_class = slv2_value_new_uri(world, SLV2_PORT_CLASS_OUTPUT);
	control_class = slv2_value_new_uri(world, SLV2_PORT_CLASS_CONTROL);

	for (i=0; i<slv2_plugin_get_num_ports(this->plugin); i++) {
		SLV2Port port = slv2_plugin_get_port_by_index(this->plugin, i);
		SLV2Value symbol = slv2_port_get_symbol(this->plugin, port);
		const char* symbol_str = slv2_value_as_string(symbol);
		this->portnames[i]=strdup(symbol_str);
		TRACE("port %s\n",symbol_str);

		if (slv2_port_is_a(this->plugin, port, control_class)) {
			TRACE("it's a control port\n");
			if (slv2_port_is_a(this->plugin, port, input_class)) {
				TRACE("control input\n");
				SLV2Value def;
				slv2_port_get_range(this->plugin, port, &def, NULL, NULL);
				this->controlvalues[i] = slv2_value_as_float(def);
				TRACE("-- Set %s to %f\n", symbol_str, this->controlvalues[i]);
				slv2_value_free(def);
			}

			slv2_instance_connect_port(this->instance, i, &this->controlvalues[i]);
		}

		else if (slv2_port_is_a(this->plugin, port, input_class)) {
			TRACE("it's an audio input port\n");
		}

		else if (slv2_port_is_a(this->plugin, port, output_class)) {
			TRACE("it's an audio output port\n");
		}

		else fail("Unsupported port type.");
	}

	this->leftin=get_port_number_by_name(this,inputs[0]);
	this->rightin=get_port_number_by_name(this,inputs[1]);
	this->leftout=get_port_number_by_name(this,outputs[0]);
	this->rightout=get_port_number_by_name(this,outputs[1]);

	TRACE("left in: %d\n",this->leftin);
	TRACE("right in: %d\n",this->rightin);
	TRACE("leftout in: %d\n",this->leftout);
	TRACE("right in: %d\n",this->rightout);

	this->leftinbuf=0;
	this->rightinbuf=0;
	this->leftoutbuf=0;
	this->rightoutbuf=0;
	this->bufsize=0;

	slv2_instance_activate(this->instance);

	this->this.get_numsamples_func=get_numsamples;
	this->this.get_samples_func=get_samples;
	this->this.set_streampos_func=set_streampos;
	this->this.dispose_func=dispose;

	this->source=source;

	set_streampos((AUDIOSOURCE *)this,0);

	return this;
}
