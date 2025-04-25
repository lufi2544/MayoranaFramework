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
#define local static
#define internal_f static

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])


////////////////////////////////////
/////// MEMORY ARENA //////////////

// NOTE: PART


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
internal_f void Mayorana_Init_Memory();


////// Entroy point for the Framework. Must call when initializing the application.
global void
Mayorana_Framework_Init()
{	
	MAYORANA_LOG("---- Framework Init----");
    Mayorana_Init_Memory();
}

internal_f void
Mayorana_Init_Memory()
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
Mayorana_Framework_End()
{
	if(g_memory.permanent.data)
	{
		VirtualFree(g_memory.permanent.data, 0, MEM_RELEASE);
	}
	
	if(g_memory.transient.data)
	{
		VirtualFree(g_memory.transient.data, 0, MEM_RELEASE);
	}
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

#define CONTROLLED_SCRATCH() \
scratch_t scratch;       \
scratch_begin(&scratch, false);    \
arena_t* temp_arena = scratch.arena; \

#endif // __cplusplus


#define SCRATCH() \
scratch_t scratch;       \
scratch_begin(&scratch, &g_memory.transient, true);    \
arena_t* temp_arena = scratch.arena; \

#define SCRATCH_END() scratch_end(&scratch);



/////////////////////////////////////////////
/////////// DATA STRUCTURES ////////////////

// NOTE: PART

///// LINKED LIST IMPLEMENTATION /////


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
for(list_node_t *node = list.head; node != 0 && (variable = LIST_NODE_DATA(node, type)); node = node->next_sibling)




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

/// LIST SORTING ///


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

/* Compare function definition. Possible outcomes:
*  |-1 a < b|
*  |0 a == b|
*  |1 a > b |
*/
typedef s8 (*list_compare_fn)(const void*, const void*);

global list_node_t*
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


global void
merge_sort(list_node_t **_head_ref, list_compare_fn compare)
{
	list_node_t *head = *_head_ref;
	if(!head || !head->next_sibling == 0)
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

//NOTE: PART 

u32 cstr_len(const char* _str)
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


///// BUFFER FORWARD DECLARATIONS

global bool buffer_is_equal(struct buffer_t a, struct buffer_t b);
global bool buffer_is_equal_cstring(struct buffer_t a, const char* b);

/////

/** This acts as a memory container of data. */
typedef struct buffer_t
{
    u8* bytes;
    u32 size;
	
#ifdef __cplusplus
    inline bool operator==(const buffer_t& other) const 
	{
        return buffer_is_equal(*this, other);
    }
	
    inline bool operator!=(const buffer_t& other) const 
	{
        return !(*this == other);
    }
	
    inline u8& operator[](u32 index) 
	{
        assert(index < size && "buffer_t index out of bounds");
        return bytes[index];
    }
	
    inline const u8& operator[](u32 index) const
	{
        assert(index < size && "buffer_t index out of bounds");
        return bytes[index];
    }
#endif // __cplusplus
	
} buffer_t;


global bool 
is_in_bounds(buffer_t _buffer, u64 _at)
{
	b32 result = (_at < _buffer.size);
	return result;
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
	
	result.bytes = (u8*)allocated_data;
	result.size = _size;
	
	return result;
}

global buffer_t 
create_buffer_string(arena_t *_arena, u32 _size)
{
	buffer_t result = create_buffer(_arena, _size);
	u8 *bytes_as_u8 = (u8*)result.bytes;
	bytes_as_u8[result.size - 1] = '\0';
	
	return result;
}

global bool 
buffer_is_equal(buffer_t a, buffer_t b)
{
	if(a.size != b.size)
	{
		return false;
	}
	
	for(u64 i = 0; i < a.size; ++i)
	{
		if(a.bytes[i] != b.bytes[i])
		{
			return false;
		}
	}
	
	return true;
}



global bool 
buffer_is_equal_cstring(buffer_t a, const char* b )
{		
	if((cstr_len(b) + 1) != a.size)
	{
		return false;
	}
	
	for(u64 i = 0; i < a.size; ++i)
	{
		if(a.bytes[i] != b[i])
		{
			return false;
		}
	}
	
	return true;
}

// With memory arena this is not needed.
global void 
free_buffer(buffer_t *_buffer)
{
	if(_buffer->bytes)
	{		
		_buffer->bytes = 0;
	}	
}




//////////////////////////////////
/////// STRING //////////////////

// NOTE: PART

/////// Forward Declarations ///////
global void
string_push_char(struct string_t *_str, u8 character);

global bool 
string_contains(struct string_t *_str, const char* b);

global bool 
is_equal_cstr(struct string_t *_str, const char* b);

global void
print_string(struct string_t *string);
//////


typedef struct string_t
{
    buffer_t buffer;
    u32 len;
	
#ifdef __cplusplus	
	
    // Compare with C string
    inline bool operator==(const char* other) const 
	{
        return buffer_is_equal_cstring(buffer, other);
    }
	
    // Compare with another string_t
    inline bool operator==(const string_t& other) const 
	{
        return buffer == other.buffer;
    }
	
    // Char access
    inline u8& operator[](u32 index) 
	{
        assert(index < len);
        return buffer.bytes[index];
    }
	
    inline const u8& operator[](u32 index) const 
	{
        assert(index < len);
        return buffer.bytes[index];
    }
	
    // Append character (like push_char)
    inline void operator += (char c) 
	{
        string_push_char(this, c);
    }
	
    // Optional: Convert to const char*
    inline const char* operator *() const 
	{
        return (const char*)buffer.bytes;
    }
	
	bool contains(const char* b)
	{
		string_contains(this, b);
	}
	
	void print()
	{
		print_string(this);
	}
			
#endif // __cplusplus
	
} string_t;


#define DEFAULT_EMPTY_STRING_LEN 20


/* Creates a string.
* 1. _content is not empty, so string len will mach _contentlen.
* 2. _content is not empty neither _len, then internal buffer will be _content size + _len + 1.
* 3. _content is empty but string not _len, then internal buffer will be _len + 1.
* 4. _content and _len are empty, then DEFAULT_EMPTY_STRING_LEN + 1 will be internal buffer size;
*/

// string raw default len
#define STRING(arena) make_string(arena, DEFAULT_EMPTY_STRING_LEN, 0)
// string lenght 
#define STRING_L(arena, lenght) make_string(arena, lenght, 0)
// string verbal
#define STRING_V(arena, content) make_string(arena, 0, content)
// string verbal lenght, ready to be expanded
#define STRING_VL(arena, lenght, content) make_string(arena, lenght, content)

// string content macro
#define STRING_CONTENT(s) (char*)s.buffer.bytes

global string_t
make_string(arena_t *_arena, u32 _len, const char* _content)
{
	string_t result;
	u32 content_len = cstr_len(_content);
	
	u32 string_buffer_size = 0;
	if(content_len > 0)
	{				
		string_buffer_size = (_len > 0) ? (content_len + _len) : (content_len);	
	}
	else
	{
		if(_len == 0)
		{
			_len = DEFAULT_EMPTY_STRING_LEN;
		}
		
		string_buffer_size = _len;
	}
	
	// null operator;
	string_buffer_size += 1;
	
	buffer_t buffer_str = create_buffer_string(_arena, string_buffer_size);
	
	result.buffer = buffer_str;
	result.len = 0;
	
	if(content_len > 0)
	{
		assert(content_len + 1 == buffer_str.size);
		for(u32 i = 0; i < content_len; ++i)
		{
			buffer_str.bytes[i] = _content[i];
		}		
		
		result.len = content_len;
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
	assert((_str->len + 1) < (_str->buffer.size));
	
    _str->buffer.bytes[_str->len++] = character;
    _str->buffer.bytes[_str->len] = '\0';		
}

global bool 
is_equal_cstr(string_t *_str, const char* b)
{
	
	return buffer_is_equal_cstring(_str->buffer, b);
}

global bool 
string_contains(string_t *_str, const char* b)
{	
	u32 c_str_len = cstr_len(b);
	if((_str == 0) || (c_str_len == 0) || (_str->len < c_str_len))
	{
		return false;
	}
	
	for(u32 i = 0; i < _str->len; ++i)
	{
		if(_str->buffer.bytes[i] == b[0])
		{
			bool bContains = true;
			// start comparing potential string
			for(u32 j = 1; j < c_str_len; ++j)
			{
				if(++i >= _str->len)
				{
					// _str has ended and the comparison could not end.
					return false;
				}
				
				if(_str->buffer.bytes[i] != b[j])
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
	printf("%s \n", string->buffer.bytes);
}