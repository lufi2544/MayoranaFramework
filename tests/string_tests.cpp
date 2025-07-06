//

TEST_SUITE_BEGIN("string");

TEST_CASE("Create String, Empty")
{	
	SCRATCH();
	
	string_t string = STRING(temp_arena);
		
	CHECK_NE(string.buffer.data, nullptr);
	CHECK_EQ(string.buffer.size, DEFAULT_EMPTY_STRING_LEN + 1);
	
}

TEST_CASE("Create String, Literal")
{	
	SCRATCH();
	
	const char *Mayorana = "Mayorana";
	u32 Mayorana_size = cstr_size(Mayorana);
	string_t string = STRING_V(temp_arena, Mayorana);	
	
	CHECK_EQ(string, Mayorana);
	CHECK_EQ(string.size, Mayorana_size);
	CHECK_EQ(string.buffer.size, Mayorana_size + 1);
	
}

TEST_CASE("Create String, Literal + Ready to be expanded")
{	
	// TODO IMPLEMENT
}


TEST_CASE("String, Deep Copy")
{
	SCRATCH();
	
	string_t s = STRING_V(temp_arena, "Hello");
	string_t a = string_copy_deep(temp_arena, &s);
	
	u8* buff = (u8*)a.buffer.data;
	buff[0] = 'J';
	
	CHECK_NE(s, a);
	
}

TEST_SUITE_END;