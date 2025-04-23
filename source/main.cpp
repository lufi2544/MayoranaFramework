
#include <cstdio>
#include "mayorana.h"

int main (int, char**)
{
    
    Mayorana_Framework_Init();    
    {
        scratch_t temp;
        
        char *buffer = push_size(char, temp.arena, sizeof(char) * 6);
        snprintf(buffer, 6, "Hello");
        
        printf("%s \n", buffer);
        
    }
    
	return 0;
}