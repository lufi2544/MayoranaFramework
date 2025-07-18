
TEST_SUITE_BEGIN("Multithreading");

global int aa = 0;
void TestFunction(int *a, int val)
{
	*a = val;
}

TEST_CASE("Thread guard")
{
	int a = 0;
	int val = 20;
	{
		mthread_t thread(&TestFunction, &a, val);
		thread_guard_t t_guard(Move(thread));
	}
	
	CHECK_EQ(a, 20);
	// If no exceptions, then we are okay;
}

TEST_SUITE_END;