#ifndef __STRUCTS_H__
#define __STRUCTS_H__


typedef struct Warehouse {
    long alive_messengers;
    long gold;
    long a;
    long b;
    long c;
    long a_price;
    long b_price;
    long c_price;
} Warehouse;


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
typedef struct OrderMessage {
    long type;
    int messanger_pid;
    Order order;
} OrderMessage;


typedef struct GoldMessage {
	long type;
	Gold gold;
} GoldMessage;

#endif // __STRUCTS_H__