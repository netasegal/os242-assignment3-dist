#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

#include "kernel/crypto.h"
 
int main(void) {
  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  printf("crypto_srv: starting\n");

  if(getpid() != 2) {
    printf("crypto_srv: must be run as the second process\n");
    exit(1);
  }

  while(1){
    struct crypto_op* va;
    uint64 size;
    printf("waiting for request\n");
    int ret = take_shared_memory_request((void**) &va, &size);
    if (ret != 0) break;
    
    //check size
    if(size != sizeof(struct crypto_op) + va->key_size + va->data_size){
      printf("crypto_srv: invalid size\n");
      exit(1);
    }

    printf("crypto_srv: received request\n");
    //check state
    if(va->state != CRYPTO_OP_STATE_INIT){
      asm volatile ("fence rw,rw" : : : "memory"); //fence
      va->state = CRYPTO_OP_STATE_ERROR;
      printf("crypto_srv: invalid state\n");
      exit(1);
    }

    //check type
    if(va->type != CRYPTO_OP_TYPE_ENCRYPT && va->type != CRYPTO_OP_TYPE_DECRYPT){
      asm volatile ("fence rw,rw" : : : "memory"); //fence
      va->state = CRYPTO_OP_STATE_ERROR;
      printf("crypto_srv: invalid type\n");
      exit(1);
    }

    printf("crypto_srv: processing request\n");
    //decrypt or decrypt
    for(int i = 0; i < va->data_size; i++){
        va->payload[va->key_size + i] ^= va->payload[i % va->key_size];
    }

    printf("crypto_srv: request processed\n");

    //when done decrypting, set state to done
    asm volatile ("fence rw,rw" : : : "memory"); //fence
    va->state = CRYPTO_OP_STATE_DONE;
    remove_shared_memory_request(va, size);
  }
  printf("crypto_srv: take_shared_memory_request failed\n");
  exit(0);
}
