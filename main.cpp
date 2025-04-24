
#include <cstdio>
#include "mayorana.h"

struct Animal
{	
	u8 type;
	char* name;
};

#ifdef MAYORANA
int main (int, char**)
{
    
    Mayorana_Framework_Init();    
    {
        scratch_t temp;
        
        char *buffer = push_array(temp.arena, 6, char);
        snprintf(buffer, 6, "Hello");
        
        printf("%s \n", buffer);
		
		int name_size = 10;
		char* animal_name_1 = push_array(temp.arena, name_size, char);
		snprintf(animal_name_1, name_size, "LOL");
		
		char* animal_name_2 = push_array(temp.arena, name_size, char);
		snprintf(animal_name_2, name_size, "LOLO");
		
		
		list_t animal_list = LIST(temp.arena);
		
		Animal a;
		a.name = animal_name_1;
		a.type = 1;
		
		Animal b;
		b.name = animal_name_2;
		b.type = 0;
		
		
		LIST_ADD(temp.arena, animal_list, a, Animal);
		LIST_ADD(temp.arena, animal_list, b, Animal);
		
		list_node_t* it = animal_list.head;
		
		while(it != 0)
		{
			Animal* animal = LIST_NODE_DATA(it, Animal);
			printf("Type: %i, name %s \n", animal->type, animal->name);
			it = it->next_sibling;
		}
		
        
    }
    
	return 0;
}
#endif // MAYORANA