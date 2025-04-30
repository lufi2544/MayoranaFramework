//

TEST_SUITE_BEGIN("buffer");


TEST_CASE("Create Buffer")
{
	SCRATCH();
	u64 elements = 255;	
	buffer_t buffer = create_buffer(temp_arena, sizeof(u32) * elements);
		
	CHECK_NE(buffer.data, nullptr);
	CHECK_EQ(buffer.size, elements * sizeof(u32));	
}

TEST_CASE("Buffer Equal")
{
	SCRATCH();
	u64 elements = 10;	
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements);
	buffer_t buffer_b = create_buffer(temp_arena, sizeof(u32) * elements);
	
	CHECK_EQ(buffer_a, buffer_b);	
}

TEST_CASE("Buffer Not Equal, Size")
{
	SCRATCH();
	u64 elements = 10;
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements + 1);
	buffer_t buffer_b = create_buffer(temp_arena, sizeof(u32) * elements);
	
	CHECK_NE(buffer_a, buffer_b);		
}

TEST_CASE("Buffer Not Equal, Content")
{
	SCRATCH();
	u64 elements = 10;
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements + 1);
	u8 *buffer_data_a = (u8*)buffer_a.data;
	buffer_data_a[0] = 22;
	
	buffer_t buffer_b = create_buffer(temp_arena, sizeof(u32) * elements);
	
	CHECK_NE(buffer_a, buffer_b);
}

TEST_CASE("Buffer Not in range")
{
	SCRATCH();
	u64 elements = 10;
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements);
	u8 index = 22;
	
	CHECK_FALSE(is_in_bounds(buffer_a, sizeof(u32) * index));
}

TEST_CASE("Buffer Equal, Content")
{
	SCRATCH();
	u64 elements = 10;
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements);
	
	u8 index = 9;
	
	CHECK_EQ(is_in_bounds(buffer_a, sizeof(u32) * index), true);
}

TEST_CASE("Free Buffer")
{
	SCRATCH();
	u64 elements = 10;	
	buffer_t buffer_a = create_buffer(temp_arena, sizeof(u32) * elements);
	free_buffer(&buffer_a);
	
	CHECK_EQ(buffer_a.data, nullptr);	
}


TEST_SUITE_END();