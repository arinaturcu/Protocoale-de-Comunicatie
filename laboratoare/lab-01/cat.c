#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void fatal(char *error_message) {
    perror(error_message);
    exit(1);
}

int main(int argc, char* argv[]) {
    int source, destination;
    int copied;
    char buf[1024];

    source = open(argv[1], O_RDONLY);

    if (source < 0) {
        fatal("File coudn't be open\n");
    }

    lseek(source, 0, SEEK_SET);

    while (copied = read(source, buf, sizeof(buf))) {
        if (copied < 0) {
            fatal("Error ar reading");
        }

        if (write(1, buf, copied) < 0) {
            fatal("Error ar writing");
        }
    }
    
    close(source);
    return 0;
}
