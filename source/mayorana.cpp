
#ifdef MAYORANA
#include <cstdio>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "mayorana.h"
#include "mayorana_tests.h"

int main (int arg_num, char** args)
{ 
	mayorana_init();
	return doctest::Context(arg_num, args).run();			
 }
 #endif // MAYORANA