#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global memory pool
static MemoryPool memory_pool = {0};
static MemoryStats memory_stats = {0};

void init_memory_manager(size_t pool_size)
{
    // Allocate memory pool
    memory_pool.start = malloc(pool_size);
    if (!memory_pool.start)
    {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(1);
    }

    // Initialize pool properties
    memory_pool.total_size = pool_size;
    memory_pool.used_size = 0;

    // Create initial free block
    memory_pool.blocks = malloc(sizeof(MemoryBlock));
    memory_pool.blocks->address = memory_pool.start;
    memory_pool.blocks->size = pool_size;
    memory_pool.blocks->is_free = true;
    memory_pool.blocks->next = NULL;

    // Initialize statistics
    memset(&memory_stats, 0, sizeof(MemoryStats));
}

static MemoryBlock *find_free_block(size_t size)
{
    MemoryBlock *current = memory_pool.blocks;
    while (current)
    {
        if (current->is_free && current->size >= size)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void split_block(MemoryBlock *block, size_t size)
{
    if (block->size > size + sizeof(MemoryBlock) + 32)
    { // Min block size = 32 bytes
        size_t remaining_size = block->size - size;
        void *split_addr = (char *)block->address + size;

        MemoryBlock *new_block = malloc(sizeof(MemoryBlock));
        new_block->address = split_addr;
        new_block->size = remaining_size;
        new_block->is_free = true;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

void *shell_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    MemoryBlock *block = find_free_block(size);
    if (!block)
    {
        fprintf(stderr, "Memory allocation failed: No free blocks available\n");
        return NULL;
    }

    // Split block if it's too large
    split_block(block, size);

    // Update block status
    block->is_free = false;

    // Update statistics
    memory_stats.total_allocated += size;
    memory_stats.current_usage += size;
    memory_stats.allocation_count++;
    if (memory_stats.current_usage > memory_stats.peak_usage)
    {
        memory_stats.peak_usage = memory_stats.current_usage;
    }

    return block->address;
}

static MemoryBlock *find_block(void *ptr)
{
    MemoryBlock *current = memory_pool.blocks;
    while (current)
    {
        if (current->address == ptr)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void merge_free_blocks(void)
{
    MemoryBlock *current = memory_pool.blocks;
    while (current && current->next)
    {
        if (current->is_free && current->next->is_free)
        {
            // Merge blocks
            current->size += current->next->size;
            MemoryBlock *to_delete = current->next;
            current->next = current->next->next;
            free(to_delete);
        }
        else
        {
            current = current->next;
        }
    }
}

void shell_free(void *ptr)
{
    if (!ptr)
        return;

    MemoryBlock *block = find_block(ptr);
    if (!block)
    {
        fprintf(stderr, "Invalid pointer passed to shell_free\n");
        return;
    }

    // Update statistics
    memory_stats.total_freed += block->size;
    memory_stats.current_usage -= block->size;
    memory_stats.free_count++;

    // Mark block as free
    block->is_free = true;

    // Merge adjacent free blocks
    merge_free_blocks();
}

void *shell_realloc(void *ptr, size_t new_size)
{
    if (!ptr)
        return shell_malloc(new_size);
    if (new_size == 0)
    {
        shell_free(ptr);
        return NULL;
    }

    MemoryBlock *block = find_block(ptr);
    if (!block)
    {
        fprintf(stderr, "Invalid pointer passed to shell_realloc\n");
        return NULL;
    }

    // Align new size to 8 bytes
    new_size = (new_size + 7) & ~7;

    // If current block is big enough, split it
    if (block->size >= new_size)
    {
        split_block(block, new_size);
        return ptr;
    }

    // Allocate new block
    void *new_ptr = shell_malloc(new_size);
    if (!new_ptr)
        return NULL;

    // Copy data and free old block
    memcpy(new_ptr, ptr, block->size);
    shell_free(ptr);

    return new_ptr;
}

void print_memory_stats(void)
{
    printf("\nMemory Manager Statistics:\n");
    printf("-------------------------\n");
    printf("Total Allocated: %zu bytes\n", memory_stats.total_allocated);
    printf("Total Freed: %zu bytes\n", memory_stats.total_freed);
    printf("Current Usage: %zu bytes\n", memory_stats.current_usage);
    printf("Peak Usage: %zu bytes\n", memory_stats.peak_usage);
    printf("Allocation Count: %zu\n", memory_stats.allocation_count);
    printf("Free Count: %zu\n", memory_stats.free_count);
    printf("-------------------------\n");
}

void print_memory_blocks(void)
{
    printf("\nMemory Blocks:\n");
    printf("-------------\n");
    MemoryBlock *current = memory_pool.blocks;
    int block_count = 0;
    while (current)
    {
        printf("Block %d: Address=%p, Size=%zu, Status=%s\n",
               ++block_count, current->address, current->size,
               current->is_free ? "Free" : "Used");
        current = current->next;
    }
    printf("-------------\n");
}

MemoryStats get_memory_stats(void)
{
    return memory_stats;
}

bool check_memory_leaks(void)
{
    size_t leaked_bytes = memory_stats.total_allocated - memory_stats.total_freed;
    if (leaked_bytes > 0)
    {
        printf("\nMemory Leak Detected!\n");
        printf("Leaked bytes: %zu\n", leaked_bytes);
        return true;
    }
    printf("\nNo memory leaks detected.\n");
    return false;
}

void cleanup_memory_manager(void)
{
    // Free all memory blocks
    MemoryBlock *current = memory_pool.blocks;
    while (current)
    {
        MemoryBlock *next = current->next;
        free(current);
        current = next;
    }

    // Free the memory pool
    free(memory_pool.start);

    // Reset statistics
    memset(&memory_stats, 0, sizeof(MemoryStats));
    memset(&memory_pool, 0, sizeof(MemoryPool));
}