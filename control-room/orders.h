#ifndef __ORDERS_H__
#define __ORDERS_H__


typedef struct Limits {
	long a;
	long b;
	long c;
} Limits;

typedef struct Order {
	long order_number;
	long a;
	long b;
	long c;
} Order;

typedef struct Gold {
	long order_number;
	long value;
} Gold;

// structure for message queue
// type - 1 order
// order - 2 gold
typedef struct OrderMessage {
    long type;
    int messanger_pid;
    Order order;
} OrderMessage;


typedef struct GoldMessage {
	long type;
	Gold gold;
} GoldMessage;
Limits setLimits(char a[], char b[], char c[]);
void generateOrders(int order_number, Order orders[], Limits order_limits);

#endif