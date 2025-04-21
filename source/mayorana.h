/* date = April 21st 2025 11:58 am */


// This framework is written with the prupose of having a centralized API for my personal use and to practice
// the application of the philosophy that less is more and with the simplicity in mind.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;
typedef void* word_t;


typedef float f32;
typedef double f64;

#define global static
#define local static
#define function_global static
#define internal_f static

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])


////////////////////////////////////
/////// Memory arena //////////////

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/mman.h>
#include <unistd.h>
#endif // __APPLE__


#define Byte(v) v
#define Megabyte(v) 1024 * Byte(v)
#define Kilobyte(v) 1024 * Megabyte(v)
#define Gigabyte(v) 1024 * Kilobyte(v)

struct memory_arena_t
{
    u8* data;
    u64 used;
    u64 size;
    u32 temp_count;
};

struct memory_t 
{
    memory_arena_t transient_memory;
    memory_arena_t permanent_memory;
    
} g_memory;

#ifndef MAYORANA_MEMORY_TRANSIENT_SIZE
#define MAYORANA_MEMORY_TRANSIENT_SIZE Megabyte(100)
#endif // MAYORANA_MEMORY_TRANSIENT_SIZE

#ifndef MAYORANA_MEMORY_PERMANENT_SIZE
#define MAYORANA_MEMORY_PERMANENT_SIZE Gigabyte(1)
#endif // MAYORANA_MEMORY_PERMANENT_SIZE


void
Mayorana_Init_Memory()
{
    // TODO crate per platform 
#ifdef __APPLE__
    
    g_memory.transient_memory.data = (u8*)mmap(0, MAYORANA_MEMORY_TRANSIENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);
    
    g_memory.transient_memory.size = MAYORANA_MEMORY_TRANSIENT_SIZE;
    
    g_memory.permanent_memory.data = (u8*)mmap(0, MAYORANA_MEMORY_PERMANENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);
    
    g_memory.permanent_memory.size = MAYORANA_MEMORY_PERMANENT_SIZE;
#endif // __APPLE__
}

void
Mayorana_Framework_Init()
{
    Mayorana_Init_Memory();
}

struct temp_arena_t;

void
_temp_arena_end(temp_arena_t *_arena);

struct temp_arena_t
{
    temp_arena_t(bool _bStack = true)
    {
        arena = &g_memory.transient_memory;
        cached_parent_used = arena->used;
        bStack = _bStack;
        
        arena->temp_count++;
    }
    
    ~temp_arena_t()
    {
        // just a tem arena that is eneded upon stack end.
        if(bStack)
        {
            _temp_arena_end(this);
        }
    }
    
    memory_arena_t *arena;
    u64 cached_parent_used;
    bool bStack;
};

void
_temp_arena_end(temp_arena_t *_arena)
{
    _arena->arena->temp_count--;
    _arena->arena->used = _arena->cached_parent_used;
    
}

word_t 
_push_size(memory_arena_t *_arena, u64 _size)
{
    if(_arena->used + _size > _arena->size)
    {
        fprintf(stderr, "Arena with size %llu trying to allocate %llu. Not enough space.", _arena->size, _size);
        
        assert(false);
    }
    
    
    u8* data = _arena->data + _arena->used;
    _arena->used += _size;
    
    return data;
}



#define push_size(t, a, s) (t*)_push_size(a, s)