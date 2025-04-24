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

#ifdef _WIN32
#include "Windows.h"
#endif // _WIN32


#define Byte(v) v
#define Megabyte(v) 1024 * Byte(v)
#define Kilobyte(v) 1024 * Megabyte(v)
#define Gigabyte(v) 1024 * Kilobyte(v)

struct arena_t
{
    u8* data;
    u64 used;
    u64 size;
    u32 temp_count;
};

struct memory_t 
{
    arena_t transient_memory;
    arena_t permanent_memory;
    
} g_memory;

#ifndef MAYORANA_MEMORY_TRANSIENT_SIZE
#define MAYORANA_MEMORY_TRANSIENT_SIZE Megabyte(100)
#endif // MAYORANA_MEMORY_TRANSIENT_SIZE

#ifndef MAYORANA_MEMORY_PERMANENT_SIZE
#define MAYORANA_MEMORY_PERMANENT_SIZE Gigabyte(1)
#endif // MAYORANA_MEMORY_PERMANENT_SIZE


void Mayorana_Init_Memory();

void
Mayorana_Framework_Init()
{
	printf("----Mayorana Init----\n");
    Mayorana_Init_Memory();
}

void
Mayorana_Init_Memory()
{
	// TODO crate per platform 
	
#ifdef __APPLE__    
    g_memory.transient_memory.data = (u8*)mmap(0, MAYORANA_MEMORY_TRANSIENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);   
    g_memory.permanent_memory.data = (u8*)mmap(0, MAYORANA_MEMORY_PERMANENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);    
	
	printf("-- Mayorana Memory Init MacOS-- \n");
#endif // __APPLE__
	
#ifdef _WIN32
	g_memory.transient_memory.data = (u8*)VirtualAlloc(0, MAYORANA_MEMORY_TRANSIENT_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	g_memory.permanent_memory.data = (u8*)VirtualAlloc(0, MAYORANA_MEMORY_PERMANENT_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	printf("-- Mayorana Memory Init Windows -- \n");
#endif // _WIN32
	
	
	g_memory.transient_memory.size = MAYORANA_MEMORY_TRANSIENT_SIZE;
    g_memory.permanent_memory.size = MAYORANA_MEMORY_PERMANENT_SIZE;
	
	printf("Memory props: \n   Arena transient: %llu \n   Arena permanent: %llu \n", g_memory.transient_memory.size, g_memory.permanent_memory.size);
	
}

struct scratch_t;

void
_scratch_end(scratch_t *_arena);

/** Definition of a Temp Arena.
 *(juanes.rayo): just using the same name 4ed uses, for simplicity when using it along the code. 
 */
struct scratch_t
{
    scratch_t(bool _bStack = true)
    {
        arena = &g_memory.transient_memory;
        cached_parent_used = arena->used;
        bStack = _bStack;
        
        arena->temp_count++;
    }
    
    ~scratch_t()
    {
        // just a tem arena that is eneded upon stack end.
        if(bStack)
        {
            _scratch_end(this);
        }
    }
    
    arena_t *arena;
    u64 cached_parent_used;
    bool bStack;
};

void
_scratch_end(scratch_t *_arena)
{
    _arena->arena->temp_count--;
    _arena->arena->used = _arena->cached_parent_used;
    
}

word_t 
_push_size(arena_t *_arena, u64 _size)
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



#define push_struct(arena, type) (type*)_push_size(arena, sizeof(type))
#define push_array(arena, count, type) (type*)_push_size(arena, (count) * sizeof(type))
#define push_size(arena, size) _push_size(arena, size)


/////////////////////////////////////////////
/////////// DATA STRUCTURES ////////////////


///// LINKED LIST IMPLEMENTATION /////


// TODO Make iterator for the list
typedef struct list_node_t
{
	void *data;
	u32 index;
	struct list_node_t *next_sibling;
	
} list_node_t;

typedef struct
{		
	list_node_t *head;
	u32 size;
} list_t;


internal_f list_node_t*
list_add_element(arena_t *_arena, list_t *_list, void* _data, u32 _size);

internal_f list_t 
make_list(arena_t *_arena, void* _data);


#define LIST_ADD(arena, list, data, type) list_add_element(arena, &list, &data, sizeof(type))
#define LIST(arena) make_list(arena, 0)
#define LIST_NODE_DATA(list_node, type) (type*)list_node->data



internal_f list_t 
make_list(arena_t *_arena, void* _data)
{
	list_t result;	
	result.size = 0;	
	result.head = 0;
	
	if(_data)
	{
		list_node_t *node = push_struct(_arena, list_node_t);
		
		if(node)
		{
			result.head = node;
			result.size++;
		}
	}
	
	return result;
}


internal_f list_node_t*
list_add_element(arena_t *_arena, list_t *_list, void* _data, u32 _size)
{
	if(_data == 0)
	{
		return 0;
	}
	
	if(_list)
	{
		list_node_t *node = push_struct(_arena, list_node_t);
		void *node_data = push_size(_arena, _size);
		node->data = node_data;
		node->next_sibling = 0;
		node->index = _list->size;
		
		if(node_data == 0)
		{
			return 0;
		}
		
		// note: (juanes.rayo): Moving this to a .c file to use as raw copy of bytes.
		u8 *data_as_bytes = (u8*)_data;
		u8 *node_data_as_bytes = (u8*)node_data;
		for(u32 i = 0; i < _size; ++i)
		{
			node_data_as_bytes[i] = data_as_bytes[i];
		}
		
		if(node)
		{
			if(!_list->head)
			{
				_list->head = node;
			}
			else
			{
				list_node_t *prev_head = _list->head;
				node->next_sibling = prev_head;
				_list->head = node;
			}
			
			_list->size++;
		}
		
		return node;
	}
	
	return 0;
	
}
