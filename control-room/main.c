#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>

#include "orders.h"


// function declarations
long receiveGold(int msg_id);
int sendOrder(int msg_id, Order order);
int validateArgs(int argc, char* argv[]);


int main(int argc, char* argv[]) {

	// quit program if args are incorect;
	printf("Verifing arguments....\n");
	if (validateArgs(argc, argv) == -1) {
		return -1;
	}

	// get order number
	long order_number = atol(argv[2]);

	// create limits 
	// and check if the limits are valid
	printf("Setting limits....\n");
	Limits limits = setLimits(argv[3], argv[4], argv[5]);
	if (limits.a == -1) {
		printf("Invalid limits\n");
		return -1;
	}

	// create message queue
	printf("Creating queue....\n");
	int msg_id = msgget(*argv[1], IPC_CREAT | 0666);
	if (msg_id == -1) {
		printf("Error while creating message queue\n");
		perror("msgget");
		return -1;
	}

	// generate orders
	printf("Generating orders....\n");
	Order orders[order_number];
	generateOrders(order_number, orders, limits);

	// main app loop 
	long gold = 0;
	long rcv_gold = 0;
	int idx = 0;
	int status;
	while (1) {
		// send orders
		if (idx < order_number) {
			printf("Sending orders to [%d]...\n", msg_id);
			if ((status = sendOrder(msg_id, orders[idx]) == -1)) {
				printf("Error while sending order\n");
			} else 
				idx++;

			// sleep for 0.5 s
			usleep(500 * 1000);
		}

		// receive gold
		if ((rcv_gold = receiveGold(msg_id)) != 0) {
			gold += rcv_gold;
			printf("Received gold price....\n");
			printf("Gold to pay: %ld\n", gold);
		}
	}
	return 0;
}


int sendOrder(int msg_id, Order order) {

	printf("Order number: %ld, A: %ld, B: %ld, C: %ld\n",
		order.order_number,
		order.a,
		order.b,
		order.c);
		
	OrderMessage message = {};
	message.type = 1;
	message.order = order;
	int err = msgsnd(msg_id, &message, sizeof(Order), IPC_NOWAIT);
	if (err == -1) {
		printf("Value of errno: %d\n", errno);
		printf("Error while sending message\n");
		perror("msgsnd");
		return -1;
	}
	return 0;
}

long receiveGold(int msg_id) {
	GoldMessage message = {};

	int err = msgrcv(msg_id, &message, sizeof(message.gold), 2, IPC_NOWAIT);
	if (err == -1) {
		if (errno != 91) {
			perror("msgrcv");
			printf("Error while receiving message\n");
			printf("Value of errno: %d\n", errno);
		}
		return 0;
	}
	printf("Gold received: %ld\n", message.gold.value);
	return message.gold.value;
}

int validateArgs(int argc, char* argv[]) {
	// function only check if arguments exists
	// and has correct types return -1 if arguments are invalid

	// arguments names list
	const char *args_names[] = {argv[0], "key", "order_number", "max_order_A", "max_order_B", "max_order_C"};

	// check if the number of arguments is correct
	if (argc != 6) {
		printf("Usage: %s <%s> <%s> <%s> <%s> <%s>\n", 
				args_names[0],
				args_names[1],
				args_names[2],
				args_names[3],
				args_names[4],
				args_names[5]);
		return -1;
	}

	// check if the queue name is not empty string
	if (strlen(argv[1]) == 0) {
		printf("Argument <%s> is missing\n", args_names[1]);
		return -1;
	}

	// check if the arguments are numbers
	for (int parameter_id=2; parameter_id < argc; parameter_id++) {
		
		// check every latter
		for (int i=0; i < strlen(argv[parameter_id]); i++) {
			if (argv[parameter_id][i] < '0' || argv[parameter_id][i] > '9') {
				printf("Argument <%s> is invalid\n", args_names[parameter_id]);
				return -1;
			}
		}
	}

	return 0;
}