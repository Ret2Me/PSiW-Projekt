#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/errno.h>

#include "structs.h"
#include "configParser.h"
#include "messanger.h"
#define MAX_WAITING_TIME 150


int messangerMain(int private_msg_id, char name[], char global_queue_name[]) {
    printf("[*] Messenger %s started\n", name);
    
    // create / connect to new global queue
    printf("[*] %s try to connect to public IPC queue: %s\n", name, global_queue_name);
    int global_msg_id = msgget(*global_queue_name, IPC_CREAT | 0666);
    if (global_msg_id == -1) {
        printf("[!] Erorr while creating / connecting to private queue");
        printf("[!] Errno value: %d", errno);
        perror("[!] msgget: ");
        return -1;
    }
    printf("[*] %s connected %s [%d]\n", name, global_queue_name, global_msg_id);


    // main router loop
    int err;
    GoldMessage recv_gold_message = {};
    OrderMessage recv_message = {}; 
    int rcv_msg_size = sizeof(recv_message.order) + sizeof(recv_message.messanger_pid);
    int rcv_gold_size = sizeof(recv_gold_message.gold);
    unsigned int waiting_time = 0; 
    while (1) {

        // waiting for incoming order
        err = msgrcv(global_msg_id, &recv_message, rcv_msg_size, 1, IPC_NOWAIT);
        if (err == -1) {
            if (errno != 91) {
                perror("msgrcv");
                printf("Error while receiving message\n");
                printf("Value of errno: %d\n", errno);
            } else {
                sleep(1);
                if (waiting_time == MAX_WAITING_TIME)
                    exit(0);
                waiting_time++;
                printf("%d\n", waiting_time);
            }
            continue;
        }
        printOrder(recv_message, "messanger");  
        waiting_time = 0;

        // add data broker pid and send it to private IPC queue (magazin)
        recv_message.messanger_pid = getpid();
        err = msgsnd(private_msg_id, &recv_message, rcv_msg_size, 0);
        if (err == -1) {
            printf("Errno value %d", errno);
            perror("msgsnd: ");
            return -1;
        }

        // waiting for response from magazine 
        err = msgrcv(private_msg_id, &recv_gold_message, rcv_gold_size, getpid(), 0);
        if (err == -1) {
            printf("Errno value %d", errno);
            perror("msgrcv: ");
            return -1;
        }
        printGold(recv_gold_message, "messenger");

        // routing message to gloabl queue
        recv_gold_message.type = 2;
        err = msgsnd(global_msg_id, &recv_gold_message, rcv_gold_size, 0);
        if (err == -1) {
            printf("Errno value %d", errno);
            perror("msgsnd: ");
            return -1;
        }
        printf("!!! SEND GOLD MESSAGE TO GLOABL QUEUE !!!\n");
    }

    return 0;
}

void printOrder(OrderMessage message, char name[]) {
    printf("[%s] Recived message:\n"
           "----------------------\n"
           "PID: %d\n"
           "Order Number: %ld\n"
           "A: %ld\n"
           "B: %ld\n"
           "C: %ld\n"
           "----------------------\n",
           name,
           message.messanger_pid,
           message.order.order_number,
           message.order.a,
           message.order.b,
           message.order.c);
}

void printGold(GoldMessage message, char name[]){
    printf("[%s] Recived message:\n"
        "----------------------\n"
        "Type: %d\n"
        "Order Number: %ld\n"
        "Price in gold: %ld\n"
        "----------------------\n",
        name,
        message.type,
        message.gold.order_number,
        message.gold.value);
}