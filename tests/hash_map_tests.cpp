TEST_SUITE_BEGIN("Hash Map");

u32 hash_function(void* k, u32 key_size)
{
	u32 *key = (u32*)(k);
	
	return *key;
}


TEST_CASE("Hash_map_init")
{
	SCRATCH();	
	hash_map_t map = HASH_MAP(temp_arena, 10, u32, u32, hash_function);
	
	hash_bucket_t bucket = map.buckets[0];		
	CHECK_EQ(map.size, 10);
	CHECK_EQ(map.data_size, sizeof(u32));
	CHECK_EQ(map.key_size, sizeof(u32));
}


TEST_CASE("Hash_map_init, key size is the max permitted")
{
	SCRATCH();
	hash_map_t map = hash_map_create(temp_arena, 10, 0, sizeof(u32), &hash_function);
	
	hash_bucket_t bucket = map.buckets[0];		
	CHECK_EQ(map.size, 10);
	CHECK_EQ(map.data_size, sizeof(u32));
	CHECK_EQ(map.key_size, g_hash_map_max_key_size);
}

TEST_CASE("Hash map, value add, bucket at exact idx, key and data, <u32, u32> OK")
{
	SCRATCH();	
	hash_map_t map = HASH_MAP(temp_arena, 10, u32, u32, hash_function);
	
	
	
	u32 key = 0;
	u32 data = 10;	
	hash_map_add(&map, &key, sizeof(u32), &data);
	
	hash_bucket_t bucket = map.buckets[key];
	
	u32* b_key = (u32*)bucket.key;
	u32* b_data = (u32*)bucket.data;
	
	CHECK_EQ((*b_key), key);
	CHECK_EQ((*b_data), data);
}

u32 hash_function_string(void* k, u32 key_size)
{
	char* key = (char*)(k);	
	return cstr_len(key);
}

TEST_CASE("Hash Map, value add, <char*, u32>")
{
	SCRATCH();
	
	hash_map_t map = HASH_MAP_STRING(temp_arena, 10, u32, hash_function_string);
	
	char key[10] = "Hello";
	u32 data = 10;
	u32 len = cstr_len(key) + 1;
	hash_bucket_t *bucket = hash_map_add(&map, (void*)key, len, &data);
		
		
	char* b_key = (char*)bucket->key;
	u32* b_data = (u32*)bucket->data;
	
	bool equal = bytes_compare(b_key, (void*)key, len);
	
	CHECK_EQ(equal, true);
	CHECK_EQ(((*b_data) == data), true);
}


TEST_CASE("")
{
	
}

TEST_SUITE_END;