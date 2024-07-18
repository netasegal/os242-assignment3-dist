#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define MSGSIZE 8060

int main() {
    
    char* shared_memory = (char*) malloc(MSGSIZE); //allocate shared memory
    int parent_pid = getpid();

    int pid = fork();

    if(pid == 0){
        printf("Size before mapping: %d\n", sbrk(0));

        //map shared memory
        char* s_mem = (char*) map_shared_pages(parent_pid, getpid(), (uint64)shared_memory, MSGSIZE); 
        if (s_mem == 0) {
            printf("map_shared_pages failed\n");
            exit(1);
        }
        printf("Size after mapping: %d\n", sbrk(0));
        printf("virtual address of s_mem: %d\n", s_mem);

        //print to shared memory
        strcpy(s_mem, "Hello Daddy"); 
        printf("map_shared_pages succeeded: %s\n", s_mem);
        
         //unmap shared memory
        int ret = unmap_shared_pages(getpid(), (uint64) s_mem, MSGSIZE);
        if(ret == -1) {
            printf("unmap_shared_pages failed\n");
            exit(1);
        }
        printf("Size after unmapping: %d\n", sbrk(0));

        char* new_mem = (char*) malloc(MSGSIZE); //allocate more
        printf("Size after malloc: %d\n", sbrk(0));
        printf("virtual address of new_mem: %d\n", new_mem);

    } else {
        wait(0);
        printf("Parent process: %s\n", shared_memory);
    }
    exit(0);

}
