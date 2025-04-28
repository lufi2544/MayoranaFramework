
#include <cstdio>
#include "mayorana.h"

struct Animal
{	
	u8 type;
	char* name;
};

#ifdef MAYORANA

s8 compare_stuff_fn(const void* _a, const void* _b)
{
	u8* a = (u8*)_a;
	u8* b = (u8*)_b;
	printf("a: %i, b: %i \n", *a, *b);
	if( *a < *b )
	{
		return -1;
	}
	else if(*a == *b)
	{
		return 0;
	}
	else if(*a > *b)
	{
		return 1;
	}
	
	return 2;
}


int main (int, char**)
{
    
    Mayorana_Framework_Init();    
    {
		SCRATCH();
        
        char *buffer = push_array(temp_arena, 6, char);
        snprintf(buffer, 6, "Hello");
        
        printf("%s \n", buffer);
		
		int name_size = 10;
		char* animal_name_1 = push_array(temp_arena, name_size, char);
		snprintf(animal_name_1, name_size, "LOL");
		char* animal_name_2 = push_array(temp_arena, name_size, char);
		snprintf(animal_name_2, name_size, "LOLO");
		
		
		list_t animal_list = LIST(temp_arena);
		
		Animal a;
		a.name = animal_name_1;
		a.type = 1;
		
		Animal b;
		b.name = animal_name_2;
		b.type = 0;
		
		
		LIST_ADD(temp_arena, animal_list, a, Animal);
		LIST_ADD(temp_arena, animal_list, b, Animal);
		
		list_node_t* it = animal_list.head;
		
		while(it != 0)
		{
			Animal* animal = LIST_NODE_DATA(it, Animal);
			printf("Type: %i, name %s \n", animal->type, animal->name);
			it = it->next_sibling;
		}		        
		string_t name = STRING_V(temp_arena, "ouuuuu shit");
		
		name.print();
		
		list_t sorting_list = LIST(temp_arena);
		
		
		u8 a_1 = 1;
		u8 a_2 = 2;
		u8 a_3 = 3;
		u8 a_4 = 4;
		u8 a_0 = 0;
		u8 a_5 = 5;
		u8 a_6 = 6;
		u8 a_7 = 7;
		
		LIST_ADD(temp_arena, sorting_list, a_0, u8);
		LIST_ADD(temp_arena, sorting_list, a_6, u8);
		LIST_ADD(temp_arena, sorting_list, a_7, u8);
		LIST_ADD(temp_arena, sorting_list, a_2, u8);
		LIST_ADD(temp_arena, sorting_list, a_1, u8);
		LIST_ADD(temp_arena, sorting_list, a_5, u8);
		LIST_ADD(temp_arena, sorting_list, a_4, u8);
		LIST_ADD(temp_arena, sorting_list, a_3, u8);
		
		{
			PRINT_LIST(u8, sorting_list, "%i",*value);
		}
		
		merge_sort(&sorting_list.head, &compare_stuff_fn);
		
		{
			PRINT_LIST(u8, sorting_list, "%i",*value);
		}						
    }			
    
	return 0;
}
#endif // MAYORANA