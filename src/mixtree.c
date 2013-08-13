#include <jansson.h>
#include <string.h>
#include "mixtree.h"
#include "util.h"
#include "audiosource_file.h"
#include "audiosource_mix.h"
#include "audiosource_repos.h"
#include "audiosource_vol.h"
#include "audiosource_trim.h"
#include "audiosource_pan.h"
#include "audiosource_ladspa.h"
#include "envelope_constant.h"
#include "envelope_linear.h"

AUDIOSOURCE *mixtree_parse(json_t *json);

/**
 * Parse envelope.
 */
ENVELOPE *mixtree_parse_envelope(json_t *json) {
	if (json_is_number(json)) {
		float value=json_number_value(json);
		ENVELOPE_CONSTANT *constant=envelope_constant_create(value);
		return (ENVELOPE *)constant;
	}

	if (json_is_array(json)) {
		ENVELOPE_LINEAR *linear=envelope_linear_create();

		int i;
		for (i=0; i<json_array_size(json); i++) {
			json_t *knotjson=json_array_get(json,i);
			json_t *timejson=json_object_get(knotjson,"time");
			json_t *valuejson=json_object_get(knotjson,"value");

			if (!timejson || !valuejson)
				fail("each knot should be a {time: T, value: V} pair.");

			int t=json_integer_value(timejson);
			t=millis_to_samples(t);
			float v=json_number_value(valuejson);

			envelope_linear_add_knot(linear,t,v);
		}

		return (ENVELOPE *)linear;
	}

	fail("Envelope should be a number or an array.");
	return NULL;
}

/**
 * Parse trim.
 */
AUDIOSOURCE *mixtree_parse_trim(json_t *json) {
	if (!json_object_get(json,"length") || !json_is_integer(json_object_get(json,"length")))
		fail("Length not specified for trim source.");

	if (!json_object_get(json,"trim") || !json_is_integer(json_object_get(json,"trim")))
		fail("Trim not specified for trim source.");

	if (!json_object_get(json,"source"))
		fail("No source specified for trim source.");

	int length=json_integer_value(json_object_get(json,"length"));
	int trim=json_integer_value(json_object_get(json,"trim"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	return (AUDIOSOURCE *)audiosource_trim_create(source,trim,length);
}

/**
 * Parse pan.
 */
AUDIOSOURCE *mixtree_parse_pan(json_t *json) {
	if (!json_object_get(json,"pan"))
		fail("Pan not specified for pan source.");

	if (!json_object_get(json,"source"))
		fail("No source specified for pan source.");

	ENVELOPE *pan=mixtree_parse_envelope(json_object_get(json,"pan"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	return (AUDIOSOURCE *)audiosource_pan_create(source,pan);
}

/**
 * Parse volume.
 */
AUDIOSOURCE *mixtree_parse_volume(json_t *json) {
	if (!json_object_get(json,"volume"))
		fail("Volume not specified for volume source.");

	if (!json_object_get(json,"source"))
		fail("No source specified for volume source.");

	ENVELOPE *volume=mixtree_parse_envelope(json_object_get(json,"volume"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	return (AUDIOSOURCE *)audiosource_vol_create(source,volume);
}

/**
 * Parse position.
 */
AUDIOSOURCE *mixtree_parse_position(json_t *json) {
	if (!json_object_get(json,"position") || !json_is_integer(json_object_get(json,"position")))
		fail("Position not specified for position source.");

	if (!json_object_get(json,"source"))
		fail("No source specified for position source.");

	int pos=json_integer_value(json_object_get(json,"position"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	return (AUDIOSOURCE *)audiosource_repos_create(source,pos);
}

/**
 * Parse mix.
 */
AUDIOSOURCE *mixtree_parse_mix(json_t *json) {
	if (!json_object_get(json,"sources") || !json_is_array(json_object_get(json,"sources")))
		fail("No sources specified for mix.");

	json_t *json_sources=json_object_get(json,"sources");
	int numsources=json_array_size(json_sources);
	AUDIOSOURCE *sources[numsources];
	int i;

	for (i=0; i<numsources; i++)
		sources[i]=mixtree_parse(json_array_get(json_sources,i));

	return (AUDIOSOURCE *)audiosource_mix_create(sources,numsources);
}

/**
 * Parse wav.
 */
AUDIOSOURCE *mixtree_parse_wav(json_t *json) {
	if (!json_object_get(json,"filename") || !json_is_string(json_object_get(json,"filename")))
		fail("Filename not specified for wav source.");

	const char *fn=json_string_value(json_object_get(json,"filename"));

	return (AUDIOSOURCE *)audiosource_file_create((char *)fn);
}

/**
 * Parse lv2.
 */
/*AUDIOSOURCE *mixtree_parse_lv2(json_t *json) {
	if (!json_object_get(json,"uri"))
		fail("LV2 plugin uri not specified.");

	if (!json_object_get(json,"source"))
		fail("No source specified for lv2.");

	const char *uri=json_string_value(json_object_get(json,"uri"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	if (json_array_size(json_object_get(json,"inputs"))!=2)
		fail("Expected two inputs.");

	if (json_array_size(json_object_get(json,"outputs"))!=2)
		fail("Expected two outputs.");

	const char *inputs[2];
	inputs[0]=json_string_value(json_array_get(json_object_get(json,"inputs"),0));
	inputs[1]=json_string_value(json_array_get(json_object_get(json,"inputs"),1));

	const char *outputs[2];
	outputs[0]=json_string_value(json_array_get(json_object_get(json,"outputs"),0));
	outputs[1]=json_string_value(json_array_get(json_object_get(json,"outputs"),1));

	return (AUDIOSOURCE *)audiosource_lv2_create(source,(char *)uri,(char **)inputs,(char **)outputs);
}*/

/**
 * Parse ladspa.
 */
AUDIOSOURCE *mixtree_parse_ladspa(json_t *json) {
	if (!json_object_get(json,"lib"))
		fail("LADSPA plugin lib not specified.");

	if (!json_object_get(json,"plugin"))
		fail("LADSPA plugin name not specified.");

	if (!json_object_get(json,"source"))
		fail("No source specified for ladspa.");

	const char *lib_file_name=json_string_value(json_object_get(json,"lib"));
	const char *plugin_name=json_string_value(json_object_get(json,"plugin"));
	AUDIOSOURCE *source=mixtree_parse(json_object_get(json,"source"));

	AUDIOSOURCE_LADSPA *ladspa=audiosource_ladspa_create(source,(char *)lib_file_name,(char *)plugin_name);

	if (json_object_get(json,"controls")) {
		json_t *controls=json_object_get(json,"controls"), *value;
		const char *key;

		json_object_foreach (controls,key,value) {
			TRACE("setting control %s=%f\n",key,json_number_value(value));
			audiosource_ladspa_set_control(ladspa,(char *)key,mixtree_parse_envelope(value));
		}
	}

	if (json_object_get(json,"extend")) {
		int extend=json_integer_value(json_object_get(json,"extend"));
		audiosource_ladspa_set_extend_time(ladspa,extend);
	}

	return (AUDIOSOURCE *)ladspa;
}

/**
 * Parse json object into audio source.
 */
AUDIOSOURCE *mixtree_parse(json_t *json) {
	if (!json_is_object(json))
		fail("Object expected");

	if (!json_object_get(json,"type"))
		fail("Audio source type not specified., json=%s",json_dumps(json, 0));

	const char *type=json_string_value(json_object_get(json,"type"));
	if (!type)
		fail("Type tag is not string");

	if (!strcmp(type,"file"))
		return mixtree_parse_wav(json);

	if (!strcmp(type,"mix"))
		return mixtree_parse_mix(json);

	if (!strcmp(type,"position"))
		return mixtree_parse_position(json);

	if (!strcmp(type,"volume"))
		return mixtree_parse_volume(json);

	if (!strcmp(type,"pan"))
		return mixtree_parse_pan(json);

	if (!strcmp(type,"trim"))
		return mixtree_parse_trim(json);

/*	if (!strcmp(type,"lv2"))
		return mixtree_parse_lv2(json);*/

	if (!strcmp(type,"ladspa"))
		return mixtree_parse_ladspa(json);

	fail("Unknown audio source type: %s.",type);

	return NULL;
}

/**
 * Load from open file.
 */
AUDIOSOURCE *mixtree_loadf(FILE *f) {
	json_error_t error;
	json_t *json=json_loadf(f,JSON_REJECT_DUPLICATES,&error);

	if (!json)
		fail("Unable to parse json.");

	return mixtree_parse(json);
}

/**
 * Load from file path.
 */
AUDIOSOURCE *mixtree_load_file(char *filename) {
	FILE *f=fopen(filename,"rb");
	if (!f)
		fail("Unable to open file %s for input.",filename);

	AUDIOSOURCE *result=mixtree_loadf(f);
	fclose(f);
	return result;
}
