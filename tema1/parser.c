#include "parser.h"

int read_rtable(struct route_table_entry *rtable, FILE *rtable_file) {
    char line[100];
    int i = 0;

    while (fgets(line, sizeof(line), rtable_file)) {
        char *token = strtok(line, " ");
        rtable[i].prefix = inet_addr(token);

        token = strtok(NULL, " ");
        rtable[i].next_hop = inet_addr(token);

        token = strtok(NULL, " ");
        rtable[i].mask = inet_addr(token);

        token = strtok(NULL, " ");
        rtable[i].interface = atoi(token);

        i++;
    }

    return i;
}