#include "common.h"

vector<tcb_t* > *getMonitor() {
	static vector<tcb_t* > *monitor = NULL;
	if (monitor != NULL)
		return monitor;
	monitor = (vector<tcb_t* > *)malloc(sizeof(vector<tcb_t* >));
	(*monitor) = vector<tcb_t* >();
	return monitor;
}
