#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <stdbool.h>

// Memory block structure
typedef struct MemoryBlock
{
    void *address;
    size_t size;
    bool is_free;
    struct MemoryBlock *next;
} MemoryBlock;

// Memory pool structure
typedef struct MemoryPool
{
    void *start;
    size_t total_size;
    size_t used_size;
    MemoryBlock *blocks;
} MemoryPool;

// Memory statistics structure
typedef struct MemoryStats
{
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    size_t allocation_count;
    size_t free_count;
} MemoryStats;

// Memory manager functions
void init_memory_manager(size_t pool_size);
void *shell_malloc(size_t size);
void shell_free(void *ptr);
void *shell_realloc(void *ptr, size_t new_size);
void print_memory_stats(void);
void cleanup_memory_manager(void);

// Memory tracking functions
MemoryStats get_memory_stats(void);
void print_memory_blocks(void);
bool check_memory_leaks(void);

#endif // MEMORY_MANAGER_H