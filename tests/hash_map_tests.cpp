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
	CHECK_EQ(map.key_size, HASH_MAP_MAX_KEY_SIZE);
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


TEST_CASE("Hash Map, value add, <char*, u32>")
{
	SCRATCH();
	
	hash_map_t map = HASH_MAP_STRING(temp_arena, 10, u32, hash_function_string);
	
	char key[10] = "Hello";
	u32 data = 10;
	u32 len = cstr_size(key) + 1;
	hash_map_add(&map, (void*)key, len, &data);
	u32 idx = 0;
	hash_map_find_v(&map, (void*)key, len, &idx);
	
	hash_bucket_t *bucket = &map.buckets[idx];
    
    
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
	u32 len = cstr_size(key) + 1;
	hash_map_add(&map, (void*)key, len, &data);
	u32 idx = 0;
	hash_map_find_v(&map, (void*)key, len, &idx);
	
	hash_bucket_t *bucket = &map.buckets[idx];
	
	void* found_data = HASH_MAP_FIND_STRING(map, key);	
	CHECK_EQ((found_data != 0), true);				
}

TEST_CASE("(Add), bucket already occupied from past hash, probing and wrapp around.")
{
	SCRATCH();
	hash_map_t map = create_u32_hash_map(temp_arena);
	
	u32 key = 8;
	u32 data = 20;
	
	HASH_MAP_ADD(map, u32, u32, key, data);
	
	u32 key_1 = 9;
	u32 data_1 = 20;
	
	HASH_MAP_ADD(map, u32, u32, key_1, data_1);
	
	u32 key_2 = 18;
	u32 data_2 = 22;
	
	HASH_MAP_ADD(map, u32, u32, key_2, data_2);
	
	
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
	HASH_MAP_ADD(map, u32, u32, key, data);
	
	u32 key_1 = 9;
	u32 data_1 = 20;	
	HASH_MAP_ADD(map, u32, u32, key_1, data_1);
	
	bool b_removed = HASH_MAP_REMOVE(map, key);
	
	u32 key_2 = 18;
	u32 data_2 = 22;	
	HASH_MAP_ADD(map, u32, u32, key_2, data_2);
	
	
	u32 idx = 99;
	void* found_data = hash_map_find_v(&map, &key_2, sizeof(u32), &idx);		
	
	CHECK_EQ((idx == 8), true);
	CHECK_EQ((found_data != 0), true);
	
}


TEST_CASE("HashMap, insert until capacity then fail to insert more")
{
    SCRATCH();
    hash_map_t map = create_u32_hash_map(temp_arena);
	
    for (u32 i = 0; i < map.size; i++)
	{
		u32 data = i * 10;
		HASH_MAP_ADD(map, u32, u32, i, data);
		u32 *found_data = 0;
		HASH_MAP_FIND(map, u32, u32, i, found_data);
        CHECK_NE((found_data == 0), true);
	}
    
	HASH_MAP_ADD(map, u32, u32, 999, 111);
	
	
	u32 *found_data = 0;
	HASH_MAP_FIND(map, u32, u32, 999, found_data);
	
    CHECK_EQ((found_data == 0), true);
}

TEST_CASE("Add duplicate key does overwrite existing value")
{
    SCRATCH();
    hash_map_t map = create_u32_hash_map(temp_arena);
	
    u32 key = 5;
    u32 data = 100;
    u32 data2 = 200;
	
    // first add
    HASH_MAP_ADD(map, u32, u32, key, data);
	
	u32 *found_data = 0;
	HASH_MAP_FIND(map, u32, u32, key, found_data);
	
	CHECK_EQ((*found_data == data), true);
	
    // try to add same key again with different value
    u32* res = (u32*)hash_map_add(&map, &key, sizeof(u32), &data2);
    // per implementation, adding an existing key should not insert (returns 0)
    CHECK_EQ((res != 0), true);
	CHECK_EQ((*res == data2), true);
	
	
    // verify stored value is still original
    u32* found = nullptr;
    HASH_MAP_FIND(map, u32, u32, key, found);
    CHECK_NE((found == 0), true);
    CHECK_EQ((*found), data2);
}

TEST_CASE("Remove non-existing key returns false and does not affect map")
{
    SCRATCH();
    hash_map_t map = create_u32_hash_map(temp_arena);
	
    u32 key = 42;
    // remove before any insert
    bool removed = HASH_MAP_REMOVE(map, key);
    CHECK_EQ(removed, false);
	
    // insert different key and ensure still present
    u32 key2 = 1;
    u32 data = 7;
    HASH_MAP_ADD(map, u32, u32, key2, data);
	
    u32* found = nullptr;
    HASH_MAP_FIND(map, u32, u32, key2, found);
    CHECK_NE((found == 0), true);
    CHECK_EQ((*found), data);
}

TEST_CASE("Find with wrong key_size does not match")
{
    SCRATCH();
    hash_map_t map = HASH_MAP(temp_arena, 10, u32, u32, hash_function_u32);
	
    u32 key = 3;
    u32 data = 9;
    hash_map_add(&map, &key, sizeof(u32), &data);
	
    // attempt to search using smaller key size (wrong size)
    void* found_wrong = hash_map_find(&map, &key, sizeof(u16)); // incorrect key size
    CHECK_EQ((found_wrong != 0), false);
	
    // correct size still finds
    void* found_ok = hash_map_find(&map, &key, sizeof(u32));
    CHECK_EQ((found_ok != 0), true);
}

TEST_CASE("String key with embedded null bytes is stored and found when exact size provided")
{
    SCRATCH();
    // custom hash that uses raw bytes size as hash (simple)
    auto hash_raw = [](void* k, u32 key_size) -> u32 {
        u8* b = (u8*)k;
        u32 h = 0;
        for (u32 i = 0; i < key_size; ++i) { h = h * 31 + b[i]; }
        return h;
    };
	
    // create a map with explicit key size of 8 bytes
    hash_map_t map = hash_map_create(temp_arena, 16, 8, sizeof(u32), (hash_function_signature)hash_raw);
	
    // key contains an embedded NUL byte in the middle
    char key_with_nul[3] = {'a', '\0', 'b'};
    u32 key_size = 3;
    u32 data = 77;
    hash_map_add(&map, (void*)key_with_nul, key_size, &data);
	
    u32 idx = 0;
    void* found = hash_map_find_v(&map, (void*)key_with_nul, key_size, &idx);
    CHECK_EQ((found != 0), true);
    u32* found_u32 = (u32*)found;
    CHECK_EQ((*found_u32), data);
}

TEST_CASE("Small map size 1 behaviour: add, remove, re-add different key")
{
    SCRATCH();
    // map with size 1 forces all keys to same bucket
    hash_map_t map = HASH_MAP(temp_arena, 1, u32, u32, hash_function_u32);
	
    u32 keyA = 10;
    u32 dataA = 1000;
    HASH_MAP_ADD(map, u32, u32, keyA, dataA);
	
    u32* foundA = nullptr;
    HASH_MAP_FIND(map, u32, u32, keyA, foundA);
    CHECK_NE((foundA == 0), true);
    CHECK_EQ((*foundA), dataA);
	
    // remove and insert another key that hashes to same index (only one index exists)
    bool removed = HASH_MAP_REMOVE(map, keyA);
    CHECK_EQ(removed, true);
	
    u32 keyB = 20;
    u32 dataB = 2000;
    HASH_MAP_ADD(map, u32, u32, keyB, dataB);
	
    u32* foundB = nullptr;
    HASH_MAP_FIND(map, u32, u32, keyB, foundB);
    CHECK_NE((foundB == 0), true);
    CHECK_EQ((*foundB), dataB);
}

TEST_CASE("Full table insert fails and probing wraps correctly")
{
    SCRATCH();
    // small map size to force full condition
    hash_map_t map = HASH_MAP(temp_arena, 3, u32, u32, hash_function_u32);
	
    // fill all slots
    for (u32 i = 0; i < map.size; ++i)
    {
        u32 data = i + 100;
        HASH_MAP_ADD(map, u32, u32, i, data);
    }
	
    // attempt extra insert - should fail
    u32 extra_key = 999;
    u32 extra_data = 555;
    void* res = hash_map_add(&map, &extra_key, sizeof(u32), &extra_data);
    CHECK_EQ((res == 0), true);
	
}

struct mock_data
{		
	u32 id;
	u32 value;
	
	bool operator ==(const mock_data& other)
	{
		return (id == other.id) && (value == other.value);
	}
};


TEST_CASE("Hash Map with key as char* and a struct as data")
{
	
	SCRATCH();
	
	hash_map_t map = hash_map_create(temp_arena, 10, 0, sizeof(mock_data), &hash_function_string);
	
	char* key = (char*)push_size(temp_arena, 10);
	bytes_set(key, 0, 10);
	
	mock_data data;
	data.id = 10;
	data.value = 1000;
	
	hash_map_add(&map, key, 10, &data);
	
	void* found_data = hash_map_find(&map, key, 10);	
	CHECK_EQ((found_data != 0), true);
    
	mock_data* mock_found_data = (mock_data*)found_data;	
	CHECK_EQ((*mock_found_data == data), true);	
}

TEST_CASE("Hash Map string keys + struct data, multiple adds and finds")
{
	SCRATCH();
    
	hash_map_t map = hash_map_create(temp_arena, 20, 0, sizeof(mock_data), &hash_function_string);
	
	char key1[10] = "Hello";
	char key2[10] = "World";
	char key3[10] = "Mayor";
	
	mock_data d1 = { 1, 100 };
	mock_data d2 = { 2, 200 };
	mock_data d3 = { 3, 300 };
    
	hash_map_add(&map, key1, cstr_size(key1) + 1, &d1);
	hash_map_add(&map, key2, cstr_size(key2) + 1, &d2);
	hash_map_add(&map, key3, cstr_size(key3) + 1, &d3);
	
	mock_data *f1 = (mock_data*)hash_map_find(&map, key1, cstr_size(key1) + 1);
	mock_data *f2 = (mock_data*)hash_map_find(&map, key2, cstr_size(key2) + 1);
	mock_data *f3 = (mock_data*)hash_map_find(&map, key3, cstr_size(key3) + 1);
	
	CHECK_EQ((f1 != 0), true);
	CHECK_EQ((f2 != 0), true);
	CHECK_EQ((f3 != 0), true);
	
	CHECK_EQ((*f1 == d1), true);
	CHECK_EQ((*f2 == d2), true);
	CHECK_EQ((*f3 == d3), true);
}

TEST_CASE("Hash Map remove struct by string key, other elements preserved")
{
	SCRATCH();
    
	hash_map_t map = hash_map_create(temp_arena, 10, 0, sizeof(mock_data), &hash_function_string);
	
	char k1[] = "K1";
	char k2[] = "K2";
	char k3[] = "K3";
	
	mock_data d1 = { 11, 100 };
	mock_data d2 = { 22, 200 };
	mock_data d3 = { 33, 300 };
    
	hash_map_add(&map, k1, 3, &d1);
	hash_map_add(&map, k2, 3, &d2);
	hash_map_add(&map, k3, 3, &d3);
	
	bool removed = hash_map_remove(&map, k2, 3);
	CHECK_EQ(removed, true);
	
	CHECK_EQ((hash_map_find(&map, k2, 3) == 0), true);
	
	CHECK_EQ((*((mock_data*)hash_map_find(&map, k1, 3)) == d1), true);
	CHECK_EQ((*((mock_data*)hash_map_find(&map, k3, 3)) == d3), true);
}

TEST_CASE("Hash Map struct collision with string keys")
{
	SCRATCH();
	
	hash_map_t map = hash_map_create(temp_arena, 5, 0, sizeof(mock_data), &hash_function_string);
	
	char a[] = "Red";
	char b[] = "Car";
	char c[] = "Sky";
	
	mock_data da = { 10, 1000 };
	mock_data db = { 20, 2000 };
	mock_data dc = { 30, 3000 };
	
	hash_map_add(&map, a, cstr_size(a) + 1, &da);
	hash_map_add(&map, b, cstr_size(b) + 1, &db);
	hash_map_add(&map, c, cstr_size(c) + 1, &dc);
	
	mock_data *fa = (mock_data*)hash_map_find(&map, a, cstr_size(a) + 1);
	mock_data *fb = (mock_data*)hash_map_find(&map, b, cstr_size(b) + 1);
	mock_data *fc = (mock_data*)hash_map_find(&map, c, cstr_size(c) + 1);
	
	CHECK_EQ((fa != 0), true);
	CHECK_EQ((fb != 0), true);
	CHECK_EQ((fc != 0), true);
    
	CHECK_EQ((*fa == da), true);
	CHECK_EQ((*fb == db), true);
	CHECK_EQ((*fc == dc), true);
}

TEST_CASE("Hash Map struct overwrite existing key")
{
	SCRATCH();
    
	hash_map_t map = hash_map_create(temp_arena, 8, 0, sizeof(mock_data), &hash_function_string);
	
	char key[] = "Hello";
	mock_data d1 = { 44, 5000 };
	mock_data d2 = { 77, 9000 };
	
	HASH_MAP_STR_ADD(map, key, &d1);
	HASH_MAP_STR_ADD(map, key, &d2);
	
	mock_data* found = 0;
	HASH_MAP_STR_FIND(map, mock_data, key, found);
    
	CHECK_EQ((*found == d2), true);
		
}


struct mock_data_pair_t
{
	char* name;
	mock_data data;
};

bool 
check_exists(mock_data_pair_t *pairs, u32 size, char* to_check_name, mock_data *to_check_data)
{
	for(u32 idx = 0; idx < size; ++idx)
	{
		mock_data_pair_t pair = pairs[idx];
		if((pair.data == *to_check_data) && cstr_compare(pair.name, to_check_name))
		{
			return true;
		}
	}
	
	return false;
}

TEST_CASE("Hash Map struct overwrite existing key")
{
	SCRATCH();
	
	
	mock_data_pair_t pairs [] = 
	{
		{"Hello1", 	{ 44, 5000 }},
		{"Hello2",	{ 77, 9000 }},
		{"Hello3",	{ 77, 9000 }},
		{"Hello4",	{ 77, 9000 }},
	};
	    
	hash_map_t map = hash_map_create(temp_arena, 8, 0, sizeof(mock_data), &hash_function_string);		
	for(u8 idx = 0; idx < ArrayCount(pairs); ++idx)
	{
		HASH_MAP_STR_ADD(map, pairs[idx].name, &pairs[idx].data);
	}	
	
	u32 idx = 0;
	for(pair_t pair : map)
	{
		char* name = (char*)pair.key;
		mock_data *data = (mock_data*)pair.data;
		++idx;
		bool bExists = check_exists(pairs, ArrayCount(pairs), name, data);
		CHECK_EQ(bExists, true);
	}
	
}


TEST_SUITE_END;