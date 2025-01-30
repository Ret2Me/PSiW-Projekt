#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "structs.h"
#define BUFFER_SIZE 2048


int fromFile(char filename[], Warehouse *warehouse) {
    int pid = open(filename, O_RDONLY);
    if (pid == -1) {
        printf("Error while opening %s file\n", filename);
        perror("open");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    long *pointers[] = {&warehouse->a, &warehouse->a_price,
                        &warehouse->b, &warehouse->b_price,
                        &warehouse->c, &warehouse->c_price};

    // read file
    int last_addr = 0;
    int struct_pointer = 0; 
    int read_bytes = read(pid, &buffer, BUFFER_SIZE);
    if (read_bytes <= 0) {
        printf("Error while reading file");
        perror("read");
        return -1;
    }

    printf("read_bytes: %d\n", read_bytes);

    // parse file
    for (int i=0; i < read_bytes; i++) {
        // change space to null and then run atoi 
        // super simple way to parse numbers because

        if (buffer[i] == ' ' || buffer[i] == '\n') {
            // double space or space before newline
            if (last_addr == i) {
                last_addr++;
                continue;
            }

            // to many values
            if (struct_pointer == 6) {
                printf("To many parameters (remove redundant spaces and newlines)\n");
                return -1;
            }

            // parse and save value to struct 
            buffer[i] = 0;  // null
            *(pointers[struct_pointer]) = atoi(&buffer[last_addr]);
            struct_pointer++;
            last_addr = i + 1;
        } else if (buffer[i] < '0' || buffer[i] > '9') {
            // check if char is a number or space or newline
            printf("Found invalid char in the config file: \"%c\"\n", buffer[i]);
            return -1; 
        }
    }

    if (buffer[read_bytes - 1] != '\n'&& buffer[read_bytes - 1] != 0 && buffer[read_bytes - 1] != ' ') {
        if (struct_pointer == 6) {
            printf("To many parameters (remove redundant spaces and newlines)\n");
            return -1;
        }
        *(pointers[struct_pointer]) = atoi(&buffer[last_addr]);
    }

    close(pid);
    return 0;
}