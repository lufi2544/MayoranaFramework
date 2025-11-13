TEST_SUITE_BEGIN("Hash Map");

global_f u32 
hash_function_u32(void* k, u32 key_size)
{
	u32 *key = (u32*)(k);
	
	return *key;
}

global_f hash_map_t
create_u32_hash_map(arena_t *arena)
{	
	hash_map_t map = HASH_MAP(arena, 10, u32, u32, hash_function_u32);
	
	return map;
}



TEST_CASE("Hash_map_init")
{
	SCRATCH();	
	hash_map_t map = HASH_MAP(temp_arena, 10, u32, u32, hash_function_u32);
	
	hash_bucket_t bucket = map.buckets[0];		
	CHECK_EQ(map.size, 10);
	CHECK_EQ(map.data_size, sizeof(u32));
	CHECK_EQ(map.key_size, sizeof(u32));
}


TEST_CASE("Hash_map_init, key size is the max permitted")
{
	SCRATCH();
	hash_map_t map = hash_map_create(temp_arena, 10, 0, sizeof(u32), &hash_function_u32);
	
	hash_bucket_t bucket = map.buckets[0];		
	CHECK_EQ(map.size, 10);
	CHECK_EQ(map.data_size, sizeof(u32));
	CHECK_EQ(map.key_size, g_hash_map_max_key_size);
}

TEST_CASE("Hash map, value add, bucket at exact idx, key and data, <u32, u32> OK")
{
	SCRATCH();	
	hash_map_t map = HASH_MAP(temp_arena, 10, u32, u32, hash_function_u32);
	
	
	
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


TEST_CASE("(Find), added element, just find it")
{
	SCRATCH();
	hash_map_t map = HASH_MAP_STRING(temp_arena, 10, u32, hash_function_string);
	
	char key[10] = "Hello";
	u32 data = 10;
	u32 len = cstr_len(key) + 1;
	hash_bucket_t *bucket = hash_map_add(&map, (void*)key, len, &data);
	
	void* found_data = HASH_MAP_FIND_STRING(map, key);	
	CHECK_EQ((found_data != 0), true);				
}

TEST_CASE("(Add), bucket already occupied from past hash, probing and wrapp around.")
{
	SCRATCH();
	hash_map_t map = create_u32_hash_map(temp_arena);
	
	u32 key = 8;
	u32 data = 20;
	
	HASH_MAP_ADD(map, key, data);
	
	u32 key_1 = 9;
	u32 data_1 = 20;
	
	HASH_MAP_ADD(map, key_1, data_1);
	
	u32 key_2 = 18;
	u32 data_2 = 22;
	
	HASH_MAP_ADD(map, key_2, data_2);
	
	
	u32 idx = 99;
	void* found_data = hash_map_find_v(&map, &key_2, sizeof(u32), &idx);
	
	CHECK_EQ((idx == 0), true);
	CHECK_EQ((found_data != 0), true);
	
}


TEST_CASE("(Add), Add element, remove it, then add same hashed element again, took the same bucket idx")
{
	SCRATCH();
	hash_map_t map = create_u32_hash_map(temp_arena);
	
	u32 key = 8;
	u32 data = 20;	
	HASH_MAP_ADD(map, key, data);
	
	u32 key_1 = 9;
	u32 data_1 = 20;	
	HASH_MAP_ADD(map, key_1, data_1);
	
	bool b_removed = HASH_MAP_REMOVE(map, key);
	
	u32 key_2 = 18;
	u32 data_2 = 22;	
	HASH_MAP_ADD(map, key_2, data_2);
	
	
	u32 idx = 99;
	void* found_data = hash_map_find_v(&map, &key_2, sizeof(u32), &idx);		
	
	CHECK_EQ((idx == 8), true);
	CHECK_EQ((found_data != 0), true);
	
}


TEST_SUITE_END;