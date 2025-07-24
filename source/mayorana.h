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
    
} memory_t;

global memory_t g_memory;

#ifndef MAYORANA_MEMORY_TRANSIENT_SIZE
#define MAYORANA_MEMORY_TRANSIENT_SIZE Megabyte(100)
#endif // MAYORANA_MEMORY_TRANSIENT_SIZE

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
	
	MAYORANA_LOG("-- Memory Init Windows --");
#endif // _WIN32
	
	
	g_memory.transient.size = MAYORANA_MEMORY_TRANSIENT_SIZE;
    g_memory.permanent.size = MAYORANA_MEMORY_PERMANENT_SIZE;
	
	MAYORANA_LOG("\n Memory: \n Arena transient: %llu \n Arena permanent: %llu \n ----  ---- \n", g_memory.transient.size, g_memory.permanent.size);	
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
	
	/** If true, in C++ the scratch gets reset when destroyed */
    bool bStack;
	
#ifdef __cplusplus
	
	// C++ constructor/destructor
	
    scratch_t(bool _bStack = true) 
	{
        arena = &g_memory.transient;
        cached_parent_used = arena->used;
        bStack = _bStack;
        arena->temp_count++;
    }
	
    ~scratch_t() 
	{
        if (bStack) 
		{
            _scratch_end(this);
        }
    }
#endif
	
} scratch_t;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
	
	internal_f void
		scratch_begin(scratch_t *_scratch, arena_t *_arena, bool _bStack)
	{
		_scratch->arena = _arena;
		_scratch->cached_parent_used = _scratch->arena->used;
		_scratch->bStack = _bStack;
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
scratch_begin(&scratch, arena, false);    \
arena_t* temp_arena = scratch.arena; \

#endif // __cplusplus


#define SCRATCH() \
scratch_t scratch;       \
scratch_begin(&scratch, &g_memory.transient, true);    \
arena_t* temp_arena = scratch.arena; \

#define SCRATCH_END() scratch_end(&scratch);



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
global_f string_t string_copy_deep(arena_t *_arena, const string_t *_other);


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

/////////////////////////
//// Multi-Threading
/////////////////////////


typedef void(*thread_function_t)(void*);

struct thread_args
{
	string_t thread_name;
	void* user_data;
	thread_function_t user_function;
};

DWORD thread_main(LPVOID _data)
{
	thread_args* args = (thread_args*)_data;
	args->user_function(args->user_data);
	
	if (!args->thread_name.is_empty())
	{
		
#ifdef _WIN32
		wchar_t buffer[256] = {};  // fixed buffer, adjust size as needed
		int count = MultiByteToWideChar(CP_UTF8, 0, STRING_CONTENT(args->thread_name), -1, buffer, 256);
		SetThreadDescription(GetCurrentThread(), buffer);
#endif // _WIN32
		
	}
	
	return 0;
}

class mythread_t
{
public:
	mythread_t() = default;
	mythread_t(arena_t* _arena, string_t _name, thread_function_t _user_function, void* _data = nullptr)
	{
		arena = _arena;
		thread_function = _user_function;
		data = _data;
		name = _name;		
		
		start();
	};
	
	void start()
	{
		if (!thread_function)
		{
			MAYORANA_LOG("There is no function assocaited with this thread, check it...");
		}
		
		
		thread_args* args = (thread_args*)push_size(arena, sizeof(thread_args));		
		args->user_data = data;
		args->user_function = thread_function;
		args->thread_name = name;
		
		LPDWORD this_id = 0;
		Handle = CreateThread(0, 0, &thread_main, args, 0, this_id);
		if(Handle != 0)
		{
			id = this_id;
		}
	}
	
	void join()
	{
		WaitForSingleObject(Handle, INFINITE);
	}
	
	
	LPDWORD id = 0;
	HANDLE Handle = 0;
	string_t name;
	
	thread_function_t thread_function;
	void* data = 0;
	
	arena_t *arena = 0;
};



typedef std::thread mthread_t;

#define THREAD(name, ...) mthread_t name(__VA_ARGS__)

// Converts an LValue Ref to an RValue Ref.
template<typename T>
std::remove_reference_t<T>&& Move(T& val)
{
	return std::move(val);
}

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
 * Custom mutex wrapper, lock_guard like.
 */
class critical_section_t
{
	public:
	critical_section_t() = default;
	
	critical_section_t(critical_section_t const&) = delete;
	critical_section_t& operator =(critical_section_t const&) = delete;
	critical_section_t(critical_section_t&& _other) = delete;
	
	~critical_section_t()
	{
		unlock();
	}
	
	void lock()
	{
		this_mutex.lock();
	}
	
	void unlock()
	{
		this_mutex.unlock();
	}
	
	bool try_lock()
	{
		return this_mutex.try_lock();
	}
	
	mutex_t& mutex()
	{
		return this_mutex;
	}
	
	mutex_t this_mutex;
};

#endif // __cplusplus

