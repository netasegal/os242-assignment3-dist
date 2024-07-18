#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MSGSIZE 8060

int main() {
    // Allocate memory in parent process
    char* shared_memory = (char*) malloc(MSGSIZE);

    // Write the string "Hello child" to shared memory
    int parent_pid = getpid();

    // Create a child process
    int pid = fork();

    if (pid == 0) {
        // Child process
        sleep(2);
        char* s_mem = (char*) map_shared_pages(parent_pid, getpid(), (uint64)shared_memory, MSGSIZE);
        if (s_mem == 0) {
            printf("map_shared_pages failed\n");
            exit(1);
        }
        printf("map_shared_pages succeeded: %s\n", s_mem);

    } else {
        // Parent process
        strcpy(shared_memory, "Hello child");
        wait(0);
        printf("Parent process: %s\n", shared_memory);
    }

    exit(0);
}