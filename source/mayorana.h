/* date = April 21st 2025 11:58 am */


// This framework is written with the prupose of having a centralized API for my personal use and to practice
// the application of the philosophy that less is more and with the simplicity in mind.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __cplusplus

// C includes
#include <stdint.h>
#include <stdbool.h>

#else
// C++ includes
#include <cstdint>

#endif // !__cplusplus

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
#define global_f static
#define local static
#define internal_f static

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])


////////////////////////////////////
/////// MEMORY ARENA //////////////

// NOTE: PART MEMORY ARENA


#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/mman.h>
#include <unistd.h>
#endif // __APPLE__

#ifdef _WIN32
#include "Windows.h"
#endif // _WIN32


#define Byte(v) v
#define Kilobyte(v) 1024 * Byte(v)
#define Megabyte(v) 1024 * Kilobyte(v)
#define Gigabyte(v) 1024 * Megabyte(v)

typedef struct arena_t
{
	
	u8* data;
    u64 used;
    u64 size;
    u32 temp_count;
	
} arena_t;

typedef struct memory_t 
{
    arena_t transient;
    arena_t permanent;
	arena_t threads;
    
} memory_t;

global memory_t g_memory;

#ifndef MAYORANA_MEMORY_TRANSIENT_SIZE
#define MAYORANA_MEMORY_TRANSIENT_SIZE Megabyte(100)
#endif // MAYORANA_MEMORY_TRANSIENT_SIZE

#ifndef MAYORANA_MEMORY_THREAD_ARENA_SIZE
#define MAYORANA_MEMORY_THREAD_ARENA_SIZE Megabyte(500)
#endif //MAYORANA_MEMORY_THREAD_ARENA_SIZE

#ifndef MAYORANA_MEMORY_PERMANENT_SIZE
#define MAYORANA_MEMORY_PERMANENT_SIZE Gigabyte(1)
#endif // MAYORANA_MEMORY_PERMANENT_SIZE


///// LOGGING //////

#define MAYORANA_LOG(format, ...) printf("Mayorana Log:" format "\n", ##__VA_ARGS__)

//////

////// Forward declarations of Init function definitions
internal_f void mayorana_memory_init();


////// Entroy point for the Framework. Must call when initializing the application.
global void
mayorana_init()
{	
	MAYORANA_LOG("---- Framework Init----");
	mayorana_memory_init();
}

internal_f void
mayorana_memory_init()
{
	
	MAYORANA_LOG("--- MEMORY ---");
#ifdef __APPLE__    
    g_memory.transient.data = (u8*)mmap(0, MAYORANA_MEMORY_TRANSIENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);   
    g_memory.permanent.data = (u8*)mmap(0, MAYORANA_MEMORY_PERMANENT_SIZE, PROT_READ | PROT_WRITE ,MAP_ANON | MAP_PRIVATE, -1, 0);    
	
	MAYORANA_LOG("-- Memory Init MacOS--");
#endif // __APPLE__
	
#ifdef _WIN32
	g_memory.transient.data = (u8*)VirtualAlloc(0, MAYORANA_MEMORY_TRANSIENT_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	g_memory.permanent.data = (u8*)VirtualAlloc(0, MAYORANA_MEMORY_PERMANENT_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	g_memory.threads.data = (u8*)VirtualAlloc(0, MAYORANA_MEMORY_THREAD_ARENA_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	MAYORANA_LOG("-- Memory Init Windows --");
#endif // _WIN32
	
	
	g_memory.transient.size = MAYORANA_MEMORY_TRANSIENT_SIZE;
    g_memory.permanent.size = MAYORANA_MEMORY_PERMANENT_SIZE;
    g_memory.threads.size = MAYORANA_MEMORY_THREAD_ARENA_SIZE;
	
	MAYORANA_LOG("\n Memory: \n Arena transient: %llu \n Arena permanent: %llu \n Arena Multi-Thread: %llu \n  ----  ---- \n", g_memory.transient.size, g_memory.permanent.size, g_memory.threads.size);	
}


global void
mayorana_shutdown()
{
#if defined(_WIN32)
	if(g_memory.permanent.data)
	{
		VirtualFree(g_memory.permanent.data, 0, MEM_RELEASE);
	}
	
	if(g_memory.transient.data)
	{
		VirtualFree(g_memory.transient.data, 0, MEM_RELEASE);
	}
    
#elif defined(__APPLE__)
    
    if (munmap(g_memory.permanent.data, g_memory.permanent.size) != 0) 
    {
        MAYORANA_LOG("Error: mmap failed for permanent memory");
    }
    
    if (munmap(g_memory.transient.data, g_memory.transient.size) != 0) 
    {
        MAYORANA_LOG("Error: mmap failed for transient memory");
    }
    
#else
#pragma error "Platform not implemented for Mayorana Shutdown"
#endif // PLATFORMS IMPLEMENTATION
}



struct scratch_t;

#ifdef __cplusplus
extern "C" {
#endif
	internal_f void _scratch_end(struct scratch_t *scratch);	
#ifdef __cplusplus
}
#endif

/** Definition of a Temp Arena.
 *  Can be used for scope pruposes or maybe out of the current scope.
 *  (juanes.rayo): just using the same name 4ed uses, for simplicity when using it along the code. 
 */
typedef struct scratch_t 
{	
	/** Parent Memory Arena */
    arena_t* arena;
	
	/** Cached memory ptr from the parent arena when created this scratch, when ended, the parent arena is reset to this. */
    u64 cached_parent_used;
		
#ifdef __cplusplus
	
	// C++ constructor/destructor
	
    scratch_t() 
	{
        arena = &g_memory.transient;
        cached_parent_used = arena->used;
        arena->temp_count++;
    }
	
    ~scratch_t() 
	{
		_scratch_end(this);        
    }
#endif
	
} scratch_t;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
	
	internal_f void
		scratch_begin(scratch_t *_scratch, arena_t *_arena)
	{
		_scratch->arena = _arena;
		_scratch->cached_parent_used = _scratch->arena->used;
		_scratch->arena->temp_count++;	
	}
	
	internal_f void
		scratch_end(scratch_t *scratch)
	{		
		_scratch_end(scratch);		
	}
	
	internal_f void
		_scratch_end(scratch_t *scratch)
	{
		if(scratch->arena)
		{
			scratch->arena->used = scratch->cached_parent_used;
			scratch->arena->temp_count--;			
			
			scratch->arena = 0;
			scratch->cached_parent_used = 0;
		}				
	}
	
	
	
#ifdef __cplusplus
}
#endif // __cplusplus


word_t 
_push_size(arena_t *_arena, u64 _size)
{
    if(_arena->used + _size > _arena->size)
    {
        fprintf(stderr, "Arena with size %llu trying to allocate %llu. Not enough space. \n", _arena->size, _size);
        
        assert(false);
    }
    
    u8* data = _arena->data + _arena->used;
    _arena->used += _size;
    
    return data;
}


#define push_struct(arena, type) (type*)_push_size(arena, sizeof(type))
#define push_array(arena, count, type) (type*)_push_size(arena, (count) * sizeof(type))
#define push_size(arena, size) _push_size(arena, size)

#ifdef __cplusplus

#define CONTROLLED_SCRATCH(arena) \
scratch_t scratch;       \
scratch_begin(&scratch, arena);    \
arena_t* temp_arena = scratch.arena; \

#endif // __cplusplus


#define SCRATCH() \
scratch_t scratch;       \
scratch_begin(&scratch, &g_memory.transient);    \
arena_t* temp_arena = scratch.arena; \


#define S_SCRATCH(memory) \
scratch_t scratch;       \
scratch_begin(&scratch, &memory->transient);    \
arena_t* temp_arena = scratch.arena; \

// Creating memory space for the threads memory that is going to be used.
#define THREADING_SCRATCH() \
scratch_t t_scratch;       \
scratch_begin(&t_scratch, &g_memory.threads);    \
arena_t* threads_arena = t_scratch.arena; \

#define SCRATCH_END() scratch_end(&scratch);
#define THREADING_SCRATCH_END() scratch_end(&t_scratch);



/////////////////////////////////////////////
/////////// DATA STRUCTURES ////////////////

// NOTE: PART DATA STRUCTURES

///// LINKED LIST IMPLEMENTATION /////
// NOTE: PART LIST

// TODO Make iterator for the list for C++
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

// TODO: create the C++ iterator overload.
#define LIST_FOREACH(type, variable, list) \
for(list_node_t *it = list.head; it != 0 && (variable = LIST_NODE_DATA(it, type)); it = it->next_sibling)
/*
 * Usage:
LIST_FOREACH(struct_t, struct_varialbe, struct_list)
{
   do stuff with "struct_variable".
}
*/

#define PRINT_LIST(type, list, format, ...) \
type* value = 0;\
LIST_FOREACH(type, value, list) \
{ \
if(it->next_sibling == 0)\
{\
printf(format "\n", ##__VA_ARGS__);\
}\
else\
{\
printf(format", ", ##__VA_ARGS__);   \
}\
}\

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

/*global list_node_t* list_get_tail(list_t *_list)
{

}*/

////////////////////
/// LIST SORTING ///

// NOTE: PART LIST SORTING

////////////////////
//// MERGE SORT ////
/* Compare function definition. Possible outcomes:
*  |-1 a < b|
*  |0 a == b|
*  |1 a > b |
*/
typedef s8 (*list_compare_fn)(const void*, const void*);

global void
merge_sort(list_node_t **_head_ref, list_compare_fn compare);

internal_f void
split_list(list_node_t *_source, list_node_t **_front_ref, list_node_t **_back_ref)
{
	list_node_t *slow = _source;
	list_node_t *fast = _source->next_sibling;
	
	// Splitting the list in 2, fast advances by 2 nodes and slow by 1, so when fast finishes the list, then slow is at the middle.
	while(fast)
	{
		fast = fast->next_sibling;
		if(fast)
		{
			slow = slow->next_sibling;
			fast = fast->next_sibling;
		}
	}
	
	*_front_ref = _source;
	*_back_ref = slow->next_sibling;
	slow->next_sibling = 0;	
}

internal_f list_node_t*
merge_sorted_lists(list_node_t *a, list_node_t *b, list_compare_fn compare)
{
	// Keep comparing the both lists separately and adding nodes as they are less than the other.
	/*
     *  a = 2, 5, 7; b = 3, 6, 8;
     *  2-3 -> add 2 => 5 - 3 -> add 3 == > 5 - 6 -> add 5 ==> 7 - 6 -> add 6 ==> 7 - 8 -> add 7 ==> 8 since a is empty. return list.
    */
	if(!a) return b;
	if(!b) return a;
	
	list_node_t *result = 0;
	
	if(compare(a->data, b->data) <= 0)
	{
		result = a;
		result->next_sibling = merge_sorted_lists(a->next_sibling, b, compare);
	}
	else
	{
		result = b;
		result->next_sibling = merge_sorted_lists(a, b->next_sibling, compare);
	}
	
	return result;
}


global_f void
merge_sort(list_node_t **_head_ref, list_compare_fn compare)
{
	list_node_t *head = *_head_ref;
	if(!head || head->next_sibling == 0)
	{
		return;
	}
	
	list_node_t *a, *b;
	split_list(head, &a, &b);
	
	merge_sort(&a, compare);
	merge_sort(&b, compare);
	
	*_head_ref = merge_sorted_lists(a, b, compare);	
}

//NOTE: PARTEND LIST


///////////////////////////////
/////// HASH MAP //////////////
/*
 For this implementation of the Hash Table, we are going to use Linear Implementation of buckets, with probing linearly when having a collision,
 and having a state flag for the buckets.

 They buckets will be a linear array with a fixed size, that means this is a Fix Sized Hash-Map as I use memory arenas as our main memory allocator.

To probe( iterate with an algorithmg and compare keys to the target one ).

Probing - Linear for now
- Add:
    - empty - then add there and mark as used.
    - stale - old used bucket, keep probing to make sure the key is not already present, remember this bucket, so we keep probing until found either an emptyslot or we reach the end of the bucket array.
    - used - just keep probing.
    
- Find:
    - Same logic as above.

- Remove:
    - Same logic as above, but in this case we mark the backet as stale and set the ptr to 0, so we dont point to old data in memory.

Probing- 
 Stop Condition - when probbed capacity buckets.
*/


// NOTE: PART HASH MAP ////


bool
bytes_compare(void *r, void *l, u32 size)
{
	u8* r_bytes = (u8*)(r);
	u8* l_bytes = (u8*)(l);
	
	for(u32 idx = 0; idx < size; ++idx)
	{
		u8 value_r = r_bytes[idx];
		u8 value_l = l_bytes[idx];
		
		if(value_r != value_l)
		{
			return false;
		}
	}
	
	return true;
}

void
bytes_set(void *mem, u32 val, u32 size)
{
	u8 *bytes = (u8*)mem;
	for(u32 idx = 0; idx < size; ++idx, bytes+=idx)
	{
		*bytes = val;
	}
}

typedef enum
{
	node_state_empty,
	node_state_occupied,
	node_state_stale
		
} my_enum_hash_node_state;


typedef struct
{	
	my_enum_hash_node_state state;
	void *key;
	void *data;
	
	u32 key_size;
	
} hash_bucket_t;

typedef u32 (*hash_function_signature)(void*);

typedef struct
{
	arena_t *arena;
	hash_bucket_t *buckets;
	hash_function_signature *hash_function;
	
	u32 size;
	u32 data_size;
		
} hash_map_t;


internal_f u32
hash_map_probe(hash_map_t *_hash_map, u32 _from_idx, void *_key, u32 _key_size, void *_data);


hash_map_t
hash_map_create(arena_t *_arena, u32 _size, hash_function_signature *_hash_function, u32 _data_size)
{
	hash_map_t result;
	
	result.arena = _arena;
	result.size = _size;
	result.data_size = _data_size;
	result.buckets = push_array(_arena, _size, hash_bucket_t);
	bytes_set(result.buckets, 0, result.data_size * sizeof(hash_bucket_t));	
	
	result.hash_function = _hash_function;
	
	return result;
}

internal_f bool
hash_map_compare_keys(hash_map_t *_map, void *_key, u32 _key_size, u32 _bucket_idx)
{
	hash_bucket_t bucket = _map->buckets[_bucket_idx];
	if(bucket.key_size != _key_size)
	{
		return false;
	}
	
	return bytes_compare(bucket.key, _key, _key_size);	
}

void* 
hash_map_find(hash_map_t *_map, void *key)
{
	return 0;
}

void 
hash_map_add(hash_map_t *_map, void* _key, u32 _key_size, void* _new_data)
{
	u32 hashed_id = (*(_map->hash_function))(_key);
	assert(_map->size != 0);	
	
	u32 bucket_idx = hashed_id % _map->size;
	assert(bucket_idx < _map->size);
	
	u32 data_size = _map->data_size;
	hash_bucket_t bucket = _map->buckets[bucket_idx];
	switch(bucket.state)
	{
		case node_state_stale:
		{
			// INIT THE DATA WITH THE SAME KEY-DATA
		}
		break;
		
		case node_state_occupied:
		{
			// if had the same key then return, else probe
			
			if(!bytes_compare(_key, bucket.key, bucket.key_size))
			{
				// probe
				u32 found_bucket_idx = hash_map_probe(_map, bucket_idx, _key, _key_size, _new_data);
				
				// this means that we have found a bucket which is fittable for our key-data.
				if(found_bucket_idx != bucket_idx)
				{
					hash_bucket_t target_backet = _map->buckets[found_bucket_idx];					
					
					// INIT THE BACKET WITH KEY-DATA
				}
			}

		}
		break;
		
		case node_state_empty:
		{
			// INIT THE BUCKET WITH KEY-DATA 
		}
		break;
		
		default: break;
	}
	
	
}



// Iterate from  bucket to bucket until we find a free one or until we find a stale one.
internal_f u32 
hash_map_probe(hash_map_t *_hash_map, u32 _from_idx, void *_key, u32 _key_size, void *_data)
{	
	u32 map_size = _hash_map->size;
	u32 possible_bucket_idx = _from_idx;
	for(u32 idx = _from_idx + 1; idx < map_size; ++idx)
	{						
		hash_bucket_t bucket = _hash_map->buckets[idx];
		switch(bucket.state)
		{
			case node_state_stale:
			{
				possible_bucket_idx= idx;
				// keep iterating to make sure we dont repeat keys;
			}
			break;			
						
			case node_state_occupied:
			{
				if(hash_map_compare_keys(_hash_map, _key, _key_size, idx))
				{
					// key already present, return this idx for comparison, to avoid returning a bool and the u32
					return _from_idx;
				} 
			}
			break;
			
			case node_state_empty:
			{
				// basically to avoid creating a bool and check if possible_bucket_idx is set
				// since we already start at possible_bucket_idx + 1 for probbing.
				if(possible_bucket_idx != _from_idx)
				{
					return possible_bucket_idx;
				}
			}
			break;
			
			default: break;
		}
	}
	
	
	return possible_bucket_idx;
}


//////////////////////////////
/////// BUFFER //////////////

//NOTE: PART BUFFER

global u32
cstr_len(const char* _str);

///// BUFFER FORWARD DECLARATIONS

global bool buffer_is_equal(struct buffer_t a, struct buffer_t b);
global void buffer_copy_deep(const struct buffer_t *_src, struct buffer_t *_dest);

/////

/**
 *  This acts as a memory container of data.
 *  It is very important to note that if we use a buffer and write to it, when creating a buffer from that memory, 
 *  the memory will NOT be 0, as we would have garbage from the previus use. 
 *
 *  Normally a buffer is used when writing some bytes to it, so we would know beforehand how many bytes we would need.
 *  But if we would use this for an array or a string, we would need to set the mem to 0 when using.
 **/
typedef struct buffer_t
{
    void* data;
    u64 size;
	
#ifdef __cplusplus
    inline bool operator==(const buffer_t& other) const 
	{
        return buffer_is_equal(*this, other);
    }
	
    inline bool operator!=(const buffer_t& other) const 
	{
        return !(*this == other);
    }	
#endif // __cplusplus
	
} buffer_t;


#define BUFFER_DATA(buffer, type, at) ((type*)buffer.data)[at]
#define BUFFER(buffer, type) (type*)buffer.data


global bool 
is_in_bounds(buffer_t _buffer, u64 _at)
{
	b32 result = (_at < _buffer.size);
	return result;
}

global_f void
buffer_copy_deep(const buffer_t *_src, buffer_t *_dest)
{
	assert(_dest->size <= _src->size);
	memcpy(_src->data, _dest->data, _dest->size);
}

global buffer_t
create_buffer(arena_t *_arena, u64 _size)
{
	buffer_t result = {0, 0};
	if(!_size)
	{
		return result;
	}	
	
	void* allocated_data  = push_size(_arena, _size);
	if(allocated_data == 0)
	{
		return result;
	}
	
	result.data = (u8*)allocated_data;
	result.size = _size;
	
	return result;
}

global bool 
buffer_is_equal(buffer_t a, buffer_t b)
{
	if(a.size != b.size)
	{
		return false;
	}
	
	u8 *a_data = (u8*)a.data;	
	u8 *b_data = (u8*)b.data;
	for(u64 i = 0; i < a.size; ++i)
	{
		if(a_data[i] != b_data[i])
		{
			return false;
		}	
	}
	
	return true;
}


// With the memory arena we only need to set the buffer to 0.
global void 
free_buffer(buffer_t *_buffer)
{
	if(_buffer->data)
	{		
		_buffer->data = 0;
	}	
}




//////////////////////////////////
/////// STRING //////////////////

// NOTE: PART STRING

/////// Forward Declarations ///////
global void
string_push_char(struct string_t *_str, u8 character);

global bool 
string_contains(struct string_t *_str, const char* b);

global bool 
is_equal_cstr(const struct string_t *_str, const char* b);

global void
print_string(struct string_t *string);
//////



global u32 
cstr_size(const char* _str)
{	
	if(_str == 0)
	{
		return 0;
	}
	
	u32 size = 0;
	char ptr = ' ';
	while(ptr != '\0')
	{
		ptr = _str[size];	
		++size;
	}
	
	return size - 1;
}


/// FORWARD DECLARATIONS ///
//global_f string_t string_copy_deep(arena_t *_arena, const string_t *_other);


typedef struct string_t
{
    buffer_t buffer;
    u32 size;
	
#ifdef __cplusplus	
	
	
	string_t() = default;		
	
	string_t(string_t const& _other)
	{
		this->buffer = _other.buffer;
		this->size = _other.size;
	}
	
	string_t(string_t&& r_value)
	{
		this->buffer = r_value.buffer;
		this->size = r_value.size;
		
		// TODO function to reset buffer.
		r_value.buffer.data = 0;
		r_value.buffer.size = 0;
		
		r_value.size = 0;
	}
	
	// Use the copy constructor or the RValue move constructor in stead.ks
	inline string_t operator=(string_t const& _other)
	{		
		this->buffer = _other.buffer;
		this->size = _other.size;
		
		return *this;
	}; 
	
	inline string_t operator= (string_t&& r_value)
	{
		this->buffer = r_value.buffer;
		this->size = r_value.size;
		r_value.buffer.data = 0;
		r_value.buffer.size = 0;
		r_value.size = 0;
		
		return *this;
	}		
	
    // Compare with C string
    inline bool operator==(const char* other) const 
	{
        return is_equal_cstr(this, other);
    }
	
    // Compare with another string_t
    inline bool operator==(const string_t& other) const 
	{
		// TODO This is not exactly right, since 
		// we can have strings with buffers with different sizes and the content is the same
		// make this content sensitive.
        return buffer == other.buffer;
    }
	
	inline bool operator!=(const string_t& other) const 
	{
		
		return !operator==(other);
    }
	
    // Char access
    inline u8& operator[](u32 index) 
	{
        assert(index < size);
		u8 *buffer_as_data = (u8*)buffer.data;
        return buffer_as_data[index];
    }
	
    inline const u8& operator[](u32 index) const 
	{
        assert(index < size);
		u8 *buffer_as_data = (u8*)buffer.data;
        return buffer_as_data[index];
    }
	
    // Append character (like push_char)
    inline void operator += (char c) 
	{
        string_push_char(this, c);
    }
	
    // Optional: Convert to const char*
    inline const char* operator *() const 
	{
        return (const char*)buffer.data;
    }
	
	bool contains(const char* b)
	{
		return string_contains(this, b);
	}
	
	bool is_empty()
	{
		return size == 0;
	}
	
	void print()
	{
		print_string(this);
	}
    
#endif // __cplusplus
	
} string_t;


#define DEFAULT_EMPTY_STRING_LEN 20


/* Creates a string.
* 1. _content is not empty, so string size will mach _contentsize.
* 2. _content is not empty neither _size, then internal buffer will be _content size + _size + 1.
* 3. _content is empty but string not _size, then internal buffer will be _size + 1.
* 4. _content and _size are empty, then DEFAULT_EMPTY_STRING_LEN + 1 will be internal buffer size;
*/

// string raw default size
#define STRING(arena) make_string(arena, DEFAULT_EMPTY_STRING_LEN, 0)
// string sizeght 
#define STRING_L(arena, size) make_string(arena, size, 0)
// string verbal
#define STRING_V(arena, content) make_string(arena, 0, content)

#define STRING_C(arena, size, format, ...) make_string_from_c_char(arena, size, format, ##__VA_ARGS__##)

// TODO: Figuring out a way of handling VL strings after creation.
// string verbal size, ready to be expanded
#define STRING_VL(arena, size, content) make_string(arena, size, content)

// string content macro
#define STRING_CONTENT(s) (char*)s.buffer.data

global string_t
make_string(arena_t *_arena, u32 _size, const char* _content)
{
	string_t result;
	u32 content_size = cstr_size(_content);
	
	u32 string_buffer_size = 0;
	if(content_size > 0)
	{				
		string_buffer_size = (_size > 0) ? (content_size + _size) : (content_size);	
	}
	else
	{
		if(_size == 0)
		{
			_size = DEFAULT_EMPTY_STRING_LEN;
		}
		
		string_buffer_size = _size;
	}
	
	// null operator;
	string_buffer_size += 1;
	
	buffer_t buffer_str = create_buffer(_arena, string_buffer_size);
	u8 *data_as_u8 = (u8*)buffer_str.data;
	
	// Reset the memory to 0 since we are using arena allocator, we can run into garbage from previous written bytes.
	memset(data_as_u8, 0, string_buffer_size);
	data_as_u8[string_buffer_size - 1] = '\0';
	
	
	result.buffer = buffer_str;
	result.size = 0;
	
	if(content_size > 0)
	{
		assert(content_size + 1 <= buffer_str.size);
		for(u32 i = 0; i < content_size; ++i)
		{
			u8 *buffer_as_data = (u8*)buffer_str.data;
			buffer_as_data[i] = _content[i];
		}
		
		result.size = content_size;
	}	
	
	return result;
}

global string_t
make_string_from_c_char(arena_t *_arena, u32 _size, const char* _format, ...)
{
	char* c_str = (char*)_push_size(_arena, _size * sizeof(char));		
	va_list args;
	va_start(args, _format);
	int chars_written = vsnprintf(c_str, _size, _format, args);
	va_end(args);		
	return STRING_V(_arena, c_str);
}

global void
string_push_char(string_t *_str, u8 character)
{
	if(_str == 0)
	{
		return;
	}
	
	// if hit, then request more memory for this string buffer upon creation.
	assert((_str->size + 1) < (_str->buffer.size));
	
	u8 *buffer_as_data = (u8*)_str->buffer.data;
	
    buffer_as_data[_str->size++] = character;
    buffer_as_data[_str->size] = '\0';		
}

global_f string_t
string_copy_deep(arena_t *_arena, const string_t *_other)
{
	string_t result;
	buffer_t this_new_buffer;
	
	void* new_data = _push_size(_arena, _other->buffer.size);
	this_new_buffer.data = new_data;
	this_new_buffer.size = _other->buffer.size;
	
	buffer_copy_deep(&_other->buffer, &this_new_buffer);
	
	result.buffer = this_new_buffer;
	result.size = _other->size;
	
	return result;	
}

// TODO TEST
global bool 
is_equal_cstr(const struct string_t *_str, const char* b)
{
	u32 c_str_size = cstr_size(b);	
	if(c_str_size != _str->size)
	{
		return false;
	}
	
	u8 *buffer_as_data = (u8*)_str->buffer.data;		
	for(u32 i = 0; i < c_str_size; ++i)
	{
		if(buffer_as_data[i] != b[i])
		{
			return false;
		}
	}
	
	return true;
}

global bool 
string_contains(string_t *_str, const char* b)
{	
	u32 c_str_size = cstr_size(b);
	if((_str == 0) || (c_str_size == 0) || (_str->size < c_str_size))
	{
		return false;
	}
	
	for(u32 i = 0; i < _str->size; ++i)
	{
		u8 *buffer_as_data = (u8*)_str->buffer.data;
		if(buffer_as_data[i] == b[0])
		{
			bool bContains = true;
			// start comparing potential string
			for(u32 j = 1; j < c_str_size; ++j)
			{
				if(++i >= _str->size)
				{
					// _str has ended and the comparison could not end.
					return false;
				}
				
				if(buffer_as_data[i] != b[j])
				{
					bContains = false;
					break;
				}
			}
			
			if(bContains)
			{
				return true;
			}			
		}
	}
	
	return true;
}

global void print_string(string_t *string)
{
	u8 *buffer_as_data = (u8*)string->buffer.data;
	printf("%s \n", buffer_as_data);
}


#ifdef __cplusplus


#include <thread>
#include <mutex>
#include <shared_mutex>

#ifdef _WIN32
#include <processthreadsapi.h>
#endif // __WIN32


// Converts an LValue Ref to an RValue Ref.
template<typename T>
std::remove_reference_t<T>&& Move(T& val)
{
	return std::move(val);
}

/////////////////////////
//// Multi-Threading
/////////////////////////
 
#include <functional>
//typedef void(*thread_function_t)(void);

typedef std::function<void()> thread_function_t;

struct thread_args
{
	string_t thread_name;
	void* user_data;
	thread_function_t user_function;
};

// We are supposed to pass-in this but the lambda should be allocated in the thread_arena.
void lambda_executor(void *lambda_ptr)
{
	thread_function_t *fn = (thread_function_t*)lambda_ptr;
	(*fn)();
}

DWORD thread_main(LPVOID _data)
{
	thread_args* args = (thread_args*)_data;	
	if (!args->thread_name.is_empty())
	{
		
#ifdef _WIN32
		wchar_t buffer[256] = {};  // fixed buffer, adjust size as needed
		int count = MultiByteToWideChar(CP_UTF8, 0, STRING_CONTENT(args->thread_name), -1, buffer, 256);
		SetThreadDescription(GetCurrentThread(), buffer);
#endif // _WIN32
		
	}
	
	args->user_function();	
	
	return 0;
}

class mythread_t
{
public:
	mythread_t() = default;
	mythread_t(mythread_t const&) = delete;	
	mythread_t& operator=(mythread_t const&) = delete;
	
	mythread_t(arena_t* _arena, string_t _name, thread_function_t _user_function)
	{
		arena = _arena;
		thread_function = _user_function;
		name = _name;		
		
		start();
	}
	
	mythread_t(mythread_t&& rvalue)
	{		
		*this = Move(rvalue);
	}
	
	mythread_t& operator= (mythread_t&& rvalue)
	{
		arena = rvalue.arena;
		thread_function = rvalue.thread_function;
		name = rvalue.name;
		id = rvalue.id;
		handle = rvalue.handle;
		
		
		rvalue.arena = 0;
		rvalue.thread_function = 0;
		rvalue.name = string_t();
		rvalue.handle = 0;
		rvalue.id = 0;
		
		return *this;
	}
	
	~mythread_t()
	{
		if(handle)
		{
			CloseHandle(handle);
			printf("Closing Handle \n");
			handle = 0;
		}
				
	}
	
	void start()
	{
		if (!thread_function)
		{
			MAYORANA_LOG("There is no function assocaited with this thread, check it...");
		}
		
		thread_args* args = (thread_args*)push_size(arena, sizeof(thread_args));		
		args->user_function = thread_function;
		args->thread_name = name;
		
		LPDWORD this_id = 0;
		handle = CreateThread(0, 0, &thread_main, args, 0, this_id);
		if(handle != 0)
		{
			id = this_id;
		}
	}
	
	void join()
	{
		WaitForSingleObject(handle, INFINITE);
	}
	
	
	LPDWORD id = 0;
	HANDLE handle = 0;
	string_t name;
	
	thread_function_t thread_function;	
	arena_t *arena = 0;
};

typedef std::thread mthread_t;

#define THREAD(name, ...) mthread_t name(__VA_ARGS__)

// NOTE: PART Multi-Threading
class thread_guard_t
{
public:
	thread_guard_t(mthread_t&& _r_thread)
	{
		this_thread = Move(_r_thread);
	}
	
	// copy const
	thread_guard_t (thread_guard_t const&) = delete;
	
	// operator =
	thread_guard_t operator=(thread_guard_t const&) = delete;
	
	~thread_guard_t()
	{
		if (this_thread.joinable())
		{
			this_thread.join();
		}
	}
	
private:
	mthread_t this_thread;
};

/////// MUTUAL EXCLUSIVE OBJECT ///////////

typedef std::mutex mutex_t;


/**
 * Custom Critical Section Wrapper wrapper, lock_guard like.
*
*  TODO: What if we delete a crit section and we have not unlocked when locked.???
*
 */
class critical_section_t
{
public:
	critical_section_t()
	{
		InitializeCriticalSection(&this_section);
	};
	
	critical_section_t(critical_section_t const&) = delete;
	critical_section_t& operator =(critical_section_t const&) = delete;
	critical_section_t(critical_section_t&& _rvalue) = delete;
	
	~critical_section_t()
	{
		DeleteCriticalSection(&this_section);
	}
	
	void lock()
	{
		EnterCriticalSection(&this_section);
	}
	
	// True if the lock has been acquired.
	bool try_lock()
	{
		return !(TryEnterCriticalSection(&this_section) == 0);
	}
	
	void unlock()
	{
		LeaveCriticalSection(&this_section);
	}
	
private:
	
#ifdef _WIN32
	CRITICAL_SECTION this_section;
#endif // _WIN32
};


// Lock wrapper for critical sections to sync the access to shared data among different threads.
class scoped_lock_t
{
public:
	scoped_lock_t(critical_section_t *_section)
	{
		this_section = _section;
		bOwnsLock = this_section->try_lock();
	}			
	
	~scoped_lock_t()
	{
		unlock();
	}
	
	void unlock()
	{
		this_section->unlock();
	}
	
	public:
	
	bool bOwnsLock = false;
	
	private:
	critical_section_t *this_section;
};


///////////////////////////////////
////////// Task Manager //////////


/* The intention of this class is to have a manager that has differentworker threads that we can use to 
 * assingn different tasks, by doing so, we can have 2 modes:
 * 
 * 1 - workers are sleeping and then we wake them up
 * 2 - workers are spinning around and when a job is detected, they take the tasks.
*/

#include <vector>
#include <functional>

volatile bool bJobsActive = true;

void JobLoop(class job_manager_t *_manager);

typedef std::function<void()> job_t;

struct job_node_t
{
	job_t job;
	s32 id = -1;
};

//// TODO, I have to reset the tail and head once the jobs have finished or find a way of resetting them, u32 wrapping?
class job_manager_t
{
	public:
	job_manager_t() = default;
	job_manager_t(const job_manager_t&) = delete;
	job_manager_t& operator = (const job_manager_t&) = delete;
	
	void init(arena_t *_arena, u32 _workers_num, u32 _max_jobs)
	{
		max_jobs = _max_jobs;
		workers_num = _workers_num;
		
		// Jobs
		jobs = (job_node_t*)push_size(_arena, sizeof(job_node_t) * max_jobs);
		workers = (mythread_t*)push_size(_arena, sizeof(mythread_t) * workers_num);
		
		// Wokers
		for(u32 i = 0; i < workers_num; ++i)
		{
			string_t name = STRING_C(_arena, 10, "worker%i", i);
			workers[i] = mythread_t(_arena,  name, [this](){ JobLoop(this); });			
		}
	}
		
	void push_job(job_t _job)
	{		
		job_node_t node;
		node.job = _job;
		node.id = tail;
		jobs[tail] = node;
			
		InterlockedIncrement(&tail);
		InterlockedIncrement(&requested_jobs);
	}
	
	void complete_job(job_node_t *_job)
	{
		printf("worker %i has finished the job \n", _job->id);
		InterlockedIncrement(&completed_jobs);
	}
	
	bool is_queue_empty()
	{
		return requested_jobs == completed_jobs && tail == head;
	}
	
	
	// Worker threads
	mythread_t *workers = 0;
	u32 workers_num = 0;
	
	// Jobs
	job_node_t *jobs = 0;
	u32 max_jobs = 0;
	volatile u32 head = 0;
	volatile u32 tail = 0;
	
	volatile u32 requested_jobs = 0;
	volatile u32 completed_jobs = 0;
	
};

void JobLoop(class job_manager_t *_manager)
{
	while(bJobsActive)
	{
		MemoryBarrier();
		if(!_manager->is_queue_empty())
		{
			job_node_t* job_node = &_manager->jobs[_manager->head];
			
			/// TODO investigate why this crashed and rely on the tail and head.
			if(job_node->job)
			{
				InterlockedIncrement(&_manager->head);
				job_node->job();
				_manager->complete_job(job_node);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}				
	}
}

#endif // __cplusplus

