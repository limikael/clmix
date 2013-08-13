#include <stdlib.h>

#include "envelope_linear.h"
#include "util.h"

typedef struct {
	int time;
	float value;
} KNOT;

struct ENVELOPE_LINEAR {
	ENVELOPE this;

	KNOT *knots;
	int numknots;
};

/**
 * Get value.
 */
static float get_value(ENVELOPE *e, int pos) {
	ENVELOPE_LINEAR *this=(ENVELOPE_LINEAR *)e;
	int i;

	if (!this->numknots)
		fail("envelope doesn't have any knots");

	if (pos<this->knots[0].time || this->numknots<2)
		return this->knots[0].value;

	for (i=0; i<this->numknots-1; i++) {
		if (pos<=this->knots[i+1].time) {
			float fpos=pos;
			float t0=this->knots[i].time, t1=this->knots[i+1].time;
			float frac=(fpos-t0)/(t1-t0);

			//TRACE("pos: %d, frac: %f, t0: %d, t1: %d\n",pos,frac,this->knots[i].time,this->knots[i+1].time);
			return this->knots[i].value+frac*(this->knots[i+1].value-this->knots[i].value);
		}
	}

	return this->knots[this->numknots-1].value;
}

/**
 * Compare knots for qsort.
 */
int knotcmp(KNOT *a, KNOT *b) {
	if (a->time>b->time)
		return 1;

	else if (a->time<b->time)
		return -1;

	else
		return 0;
}

/**
 * Add knot.
 */
void envelope_linear_add_knot(ENVELOPE_LINEAR *this, int time, float value) {
	this->knots=nicerealloc(this->knots,sizeof(KNOT)*(this->numknots+1));

	this->knots[this->numknots].time=time;
	this->knots[this->numknots].value=value;

	this->numknots++;

	qsort(this->knots,this->numknots,sizeof(KNOT),knotcmp);
}

/**
 * Constructor.
 */
ENVELOPE_LINEAR *envelope_linear_create() {
	ENVELOPE_LINEAR *this=nicemalloc(sizeof (ENVELOPE_LINEAR));

	envelope_init((ENVELOPE *)this);
	this->this.get_value_func=get_value;

	this->knots=NULL;
	this->numknots=0;

	return this;
}