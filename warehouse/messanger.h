#ifndef __MESSANGER_H__
#define __MESSANGER_H__


int messangerMain(int msg_id, char name[], char global_queue_name[]);
void printOrder(OrderMessage message, char name[]);
void printGold(GoldMessage message, char name[]);
#endif // __MESSANGER_H__