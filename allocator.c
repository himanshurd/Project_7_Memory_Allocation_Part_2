#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

void print_data(void);
void *myalloc(int bytes);

struct block {
    int in_use;
    int size;
    struct block *next;
};
struct block *head = NULL;

// Split_Space(current_node, requested_size):
void split_space(struct block *cur, int bytes){ 
  int required_space = PADDED_SIZE(bytes) + PADDED_SIZE(sizeof(struct block)) + 16;
  // Split if it can hold the three things, space requested from user, 
  // A new struct block, and the remaining space referred to by the new struct block (padded)
  if (cur->size >= required_space){
    struct block *new = PTR_OFFSET(cur, PADDED_SIZE(sizeof(struct block)));                 
    new->next = NULL;                              // Wire it into the linked list
    cur->next = new;
    new ->size = (cur->size) - PADDED_SIZE(bytes) - PADDED_SIZE(sizeof(struct block));
    cur->size = PADDED_SIZE(bytes);
  }
}

void myfree(void *cur) {                         // Free Node taking pointer from myalloc()
  struct block *node = cur;                      // create a pointer node to be freed
  node=node-1;                                   // compute location through pointer subtraction
  node->in_use = 0;                              // mark node as not in use
}

void *myalloc(int bytes){
  int actual_size = PADDED_SIZE(bytes);
  
  if (head == NULL){
    head = mmap(NULL, 1024, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE, -1, 0);;
    head->next = NULL;
    head->size = 1024 - PADDED_SIZE(sizeof(struct block));
    head->in_use = 0;
  }
  struct block *cur = head;
  while(cur != NULL){
    if ((!cur->in_use)&&(cur->size>=actual_size)){
      split_space(cur, bytes);
      cur->in_use = 1;
      int padded_block_size = PADDED_SIZE(sizeof(struct block));
      return PTR_OFFSET(cur, padded_block_size);
    }
    cur = cur->next;
  }
  return NULL;
}

void print_data(void)
{
  struct block *b = head;
  if (b == NULL) {
    printf("[empty]\n");
    return;
  }
  while (b != NULL) {
    // Uncomment the following line if you want to see the pointer values
    //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
    printf("[%d,%s]", b->size, b->in_use? "used": "free");
    if (b->next != NULL) {
      printf(" -> ");
    }
    b = b->next;
  }
  printf("\n");
}

int main(void) {
//   void *p;
//   p = myalloc(512);
//   print_data();
//   myfree(p);
//   print_data();
  
  myalloc(10); print_data();
  myalloc(20); print_data();
  myalloc(30); print_data();
  myalloc(40); print_data();
  myalloc(50); print_data();
  
//   void *p;
//   myalloc(10);     print_data();
//   p = myalloc(20); print_data();
//   myalloc(30);     print_data();
//   myfree(p);       print_data();
//   myalloc(40);     print_data();
//   myalloc(10);     print_data();
}
