#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void fatal(char *error_message) {
    perror(error_message);
    exit(1);
}

void print_from_start_to_stop(int source, int start, int stop) {
    int copied;
    char buf[1024];

    lseek(source, start, SEEK_SET);

    copied = read(source, buf, stop - start + 1);
    if (copied < 0) {
        fatal("Error ar reading");
    }
    
    if (write(1, buf, copied) < 0) {
        fatal("Error ar writing");
    }
}

int main(int argc, char* argv[]) {
    int source, destination;
    int copied;
    char buf[1024];
    int size;

    source = open(argv[1], O_RDONLY);

    if (source < 0) {
        fatal("File coudn't be open\n");
    }

    size = lseek(source, 0, SEEK_END);

    int i = 2;
    int stop = lseek(source, -i, SEEK_END) + 1;

    while ((copied = read(source, buf, 1)) && (i <= size + 3)) {
        if (copied < 0) {
            fatal("Error ar reading");
        }

        if (buf[0] == '\n') {
            print_from_start_to_stop(source, size - i + 2, stop);
            stop = size - i + 1;
        } else if ((size - i + 3) == 0) {
            print_from_start_to_stop(source, size - i + 3, stop);
            stop = size - i + 1;
        }

        lseek(source, -(i++), SEEK_END);
    }
    
    close(source);
    return 0;
}
