


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

TEST_CASE("mythread_t multiple threads")
{
    SCRATCH();
	
    const int count = 4;
    int values[count] = {};
	
    auto ThreadFunc = [](void* data)
    {
        int* ptr = (int*)data;
        *ptr += 10;
    };
	
    mythread_t threads[count];
	
    for (int i = 0; i < count; ++i)
    {
        start_thread(&threads[i],
                     temp_arena,
                     STRING_C(temp_arena, "worker%i", i),
                     ThreadFunc,
                     &values[i]);
    }
	
    for (int i = 0; i < count; ++i)
    {
        threads[i].join();
        end_thread(&threads[i]);
    }
	
    for (int i = 0; i < count; ++i)
    {
        CHECK_EQ(values[i], 10);
    }
}



//////////////////// JOB MANAGER ///////////////////////////

TEST_CASE("job_manager executes single job")
{
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 1, 16);
	
    static int result = 0;
	
    auto Job = [](void* )
    {
        result = 77;
    };
	
    manager.PushJob(Job, 0);
	
    while (IsJobManagerBusy(&manager))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	
    CHECK_EQ(result == 77, true);
	
	JobManagerShutDown(&manager);
}

TEST_CASE("job_manager executes multiple jobs single worker")
{
	
	bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 1, 64);
	
    static std::atomic<int> counter = 0;
	
    auto Job = [](void*)
    {
        counter.fetch_add(1);
    };
	
    const int jobCount = 50;
	
    for (int i = 0; i < jobCount; ++i)
    {
        manager.PushJob(Job, 0);
    }
	
	while (IsJobManagerBusy(&manager))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	
    CHECK_EQ(counter.load(), jobCount);
	
	JobManagerShutDown(&manager);
}


TEST_CASE("job_manager multiple workers stress test")
{
	bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 4, 256);
	
    static std::atomic<int> counter = 0;
	
    auto Job = [](void *)
    {
        counter.fetch_add(1);
    };
	
    const int jobCount = 200;
	
    for (int i = 0; i < jobCount; ++i)
    {
        manager.PushJob(Job, 0);
    }
	
    while (IsJobManagerBusy(&manager))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	
    CHECK_EQ(counter.load(), jobCount);
	
	
	JobManagerShutDown(&manager);
}




TEST_CASE("job_manager multiple workers stress test")
{
	bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 4, 256);
	
    static std::atomic<int> counter = 0;
	
    auto Job = [](void *)
    {
        counter.fetch_add(1);
    };
	
    const int jobCount = 200;
	
    for (int i = 0; i < jobCount; ++i)
    {
        manager.PushJob(Job, 0);
    }
	
    while (IsJobManagerBusy(&manager))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	
    CHECK_EQ(counter.load(), jobCount);
	
	
	JobManagerShutDown(&manager);
}

TEST_CASE("job_manager capacity plus one")
{
    bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    const int capacity = 16;
    manager.init(temp_arena, 2, capacity);
	
    std::atomic<int> counter{0};
	
    auto Job = [](void* data)
    {
        auto* c = (std::atomic<int>*)data;
        c->fetch_add(1);
    };
	
    for (int i = 0; i < capacity + 1; ++i)
        manager.PushJob(Job, &counter);
	
    while (manager.completed_jobs != manager.requested_jobs)
        std::this_thread::yield();
	
    JobManagerShutDown(&manager);
	
    CHECK_EQ(counter.load(), capacity + 1);
}

TEST_CASE("job_manager zero jobs")
{
    bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 2, 16);
	
    // No jobs pushed
	
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
	
    JobManagerShutDown(&manager);
	
    CHECK(true); // just ensure no deadlock
}

TEST_CASE("job_manager shutdown while busy")
{
    bJobsActive = true;
    SCRATCH();
	
    job_manager_t manager;
    manager.init(temp_arena, 4, 128);
	
    std::atomic<int> counter{0};
	
    auto Job = [](void* data)
    {
        auto* c = (std::atomic<int>*)data;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        c->fetch_add(1);
    };
	
    for (int i = 0; i < 50; ++i)
        manager.PushJob(Job, &counter);
	
    // Immediately shutdown
    JobManagerShutDown(&manager);
	
    CHECK(counter.load() <= 50);
}

TEST_SUITE_END;