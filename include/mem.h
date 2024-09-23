
#include <unistd.h>


#define MIN_BLOCK_SIZE 16
#define MAX_BLOCK_CNT 1024

typedef struct {
    void* begin;
    size_t size;
    size_t alignment;

    void* blocks[MAX_BLOCK_CNT];
} allocator_t;