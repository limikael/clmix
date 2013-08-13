#ifndef __ENVELOPE_LINEAR_H__
#define __ENVELOPE_LINEAR_H__

#include "envelope.h"

typedef struct ENVELOPE_LINEAR ENVELOPE_LINEAR;

ENVELOPE_LINEAR *envelope_linear_create();
void envelope_linear_add_knot(ENVELOPE_LINEAR *e, int time, float value);

#endif