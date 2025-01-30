#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <signal.h>

#include "structs.h"
#include "configParser.h"
#include "messanger.h"


void spawnMessengers(int msg_id, char global_queue_key[], int messenger_pids[]);
void executeOrder(int msg_id, OrderMessage order_message);
void killMessenger(int pid);
void closeWarehouse();
int lifecheckMessenger(int messenger_pids[]);


// application state should be allocated on heap
Warehouse warehouse = {};


int main(int argc, char *argv[]) {

    // verify arguments
    if (argc < 3) {
        printf("Usage %s <config_name> <global_queue_name>", argv[0]);
        return -1;
    }

    // read config
    if (fromFile(argv[1], &warehouse) == -1) {
        printf("Error while reading config\n");
        return -1;
    }

    // display app status
    printf("[*] Warehouse started\n"
           "[*] -------------------------------\n"
           "[*] Warehouse config: \n"
           "[*] A: %ld pcs | %ld price per pcs\n"
           "[*] B: %ld pcs | %ld price per pcs\n"
           "[*] C: %ld pcs | %ld price per pcs\n"
           "[*] -------------------------------\n"
           "[*] Connecting to PRIVATE queue:\n",
            warehouse.a, warehouse.a_price,
            warehouse.b, warehouse.b_price,
            warehouse.c, warehouse.c_price
    );

    // create / connect to new private queue
    int msg_id = msgget(IPC_PRIVATE, 0666);
    if (msg_id == -1) {
        printf("[!] Erorr while creating / connecting to private queue");
        printf("[!] Errno value: %d", errno);
        perror("[!] msgget: ");
        return -1;
    }
    printf("[*] Successfuly connected to internal queue.\n"
           "[*] -------------------------------\n");

    // spawn messengers
    int messenger_pids[3] = {0, 0, 0};
    spawnMessengers(msg_id, argv[2], messenger_pids);
    printf("[*] -------------------------------\n");
    
    // main app loop
    OrderMessage order_message = {};
    int message_size = sizeof(order_message.messanger_pid) + sizeof(order_message.order);
    int err, order_price;
    while (1) {

        // Check for pending orders
        err = msgrcv(msg_id, &order_message, message_size, 1, IPC_NOWAIT);
        if (err == -1) {
            if (errno != 91) {
                perror("msgrcv");
                printf("Error while receiving message\n");
                printf("Value of errno: %d\n", errno);
            }
        } else {
            // recived order
            printOrder(order_message, "warehouse");
            executeOrder(msg_id, order_message);
        }

        // messengers life check
        if (lifecheckMessenger(messenger_pids) == -1)
            closeWarehouse();

        sleep(1);
    }
    return 0;
}


void spawnMessengers(int msg_id, char global_queue_key[], int messenger_pids[]) {
    int alive_messengers = 3;
    int messanger_num = 3;
    char *messangers[] = {"Mercury", "Venus", "Saturn"};
    for (int i=0; i < messanger_num; i++) {
        if ((messenger_pids[i] = fork()) == 0) {
            messangerMain(msg_id, messangers[i], global_queue_key);
            exit(0);
        }
    }
    usleep(200);
}


void killMessenger(int pid) {
    kill(pid, SIGKILL);
}


int lifecheckMessenger(int messenger_pids[]) {
    int response; 
    int status;
    pid_t result;
    for (int i=0; i < 3; i++)
        if ((result = waitpid(messenger_pids[i], &status, WNOHANG)) == 0)
            return 0;
    return -1;
}


void closeWarehouse() {
    printf("[*] All messengers are dead here are warehouse stats:\n");
    printf("[*] ---------------------------------\n");
    printf("[*] A pcs: %ld \n", warehouse.a);
    printf("[*] B pcs: %ld \n", warehouse.b);
    printf("[*] C pcs: %ld \n", warehouse.c);
    printf("[*] gold: %ld \n", warehouse.gold);
    printf("[*] ---------------------------------\n");
    exit(0);
}


void executeOrder(int msg_id, OrderMessage order_message) {
    Order order = order_message.order;
    int order_price;

    // if not enough resources kill messenger
    if (warehouse.a < order.a  || warehouse.b < order.b || warehouse.c < order.c) {
        killMessenger(order_message.messanger_pid);
        warehouse.alive_messengers -= 1;
        if (warehouse.alive_messengers == 0)
            closeWarehouse();
        return;
    }

    order_price = order.a * warehouse.a_price 
                    + order.b * warehouse.b_price
                    + order.c * warehouse.c_price;

    // send info to messenger
    Gold gold = {};
    gold.order_number = order_message.order.order_number;
    gold.value = order_price;

    GoldMessage gold_message = {};
    gold_message.type = order_message.messanger_pid;
    gold_message.gold = gold;
    int err = msgsnd(msg_id, &gold_message, sizeof(gold_message.gold), 0);
	if (err == -1) {
		printf("Value of errno: %d\n", errno);
		printf("Error while sending message\n");
		perror("msgsnd");
		return;
	}

    // update warehouse resources
    warehouse.gold += order_price;
    warehouse.a -= order_message.order.a;
    warehouse.b -= order_message.order.b;
    warehouse.c -= order_message.order.c;
}
