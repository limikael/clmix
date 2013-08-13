#include "envelope_constant.h"
#include "util.h"

struct ENVELOPE_CONSTANT {
	ENVELOPE this;

	float value;
};

/**
 * Get value.
 */
static float get_value(ENVELOPE *e) {
	ENVELOPE_CONSTANT *this=(ENVELOPE_CONSTANT *)e;

	return this->value;
}

/**
 * Constructor.
 */
ENVELOPE_CONSTANT *envelope_constant_create(float value) {
	ENVELOPE_CONSTANT *this=nicemalloc(sizeof (ENVELOPE_CONSTANT));

	envelope_init((ENVELOPE *)this);
	this->value=value;
	this->this.get_value_func=get_value;

	return this;
}