#include <stdlib.h>
#include "orders.h"


Limits setLimits(char a[], char b[], char c[]) {
	// return limits structure with values a, b, c
	// if the values are invalid, return -1 for all values

	// convert char[] to long
	long parsed_a = atol(a);
	long parsed_b = atol(b);
	long parsed_c = atol(c);

	// check if the values are valid
	if (parsed_a < 0 || parsed_b < 0 || parsed_c < 0) {
		// return an error
		parsed_a = -1;
		parsed_b = -1;
		parsed_c = -1;
	}

	// set limits
	Limits limits;
	limits.a = parsed_a;
	limits.b = parsed_b;
	limits.c = parsed_c;
	return limits;
}


void generateOrders(int order_number, Order orders[], Limits order_limits) {
	// generate random orders and store them in the orders array

	for (int i=0; i < order_number; i++) {
		// generate random orders <0; order_limits>
		// used function from stdlib.h
		orders[i].order_number = i;
		orders[i].a = rand() % (order_limits.a + 1);
		orders[i].b = rand() % (order_limits.b + 1);
		orders[i].c = rand() % (order_limits.c + 1);
	}
}
