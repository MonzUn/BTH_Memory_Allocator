#include <new>
#include <vector>
#include <chrono>
#include <algorithm>
#include "MemoryAllocator.h"
#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include "Logger.h"

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#define THREAD_TEST_THREAD_COUNT_MAX 32
#define FRAME_TEST_FRAME_COUNT 64
#define FRAME_TEST_ITERATIONS_PER_FRAME 100000

struct DebugStruct
{
	DebugStruct() {};
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

struct GameObject
{
	GameObject() {};
	GameObject(int lifeTime) : LifeTime(lifeTime) {};

	bool DeleteMe() { LifeTime--; return (LifeTime > 0) ? false : true; }

	int		LifeTime;
};

void TestFrameAllocator(unsigned char* mallocSizes);
void TestPoolAllocator();
void TestPoolAllocator2();
void PrintHelp();

Logger LogOut;

int main()
{
	PrintHelp();

	bool quit = false;
	while (!quit)
	{
		std::cout << "> ";
		std::string input;
		std::cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);

		if (input == "quit" || input == "exit" || input == "q")
			quit = true;

		else if ( input == "frameTest" || input == "f" )
		{
			LogOut << "Starting frame allocator test\n";
			std::chrono::steady_clock::time_point start, end;
			long long duration;
			srand( 40805 );

			unsigned char* allocationSizes = static_cast<unsigned char*>( malloc( sizeof( char ) * FRAME_TEST_ITERATIONS_PER_FRAME * FRAME_TEST_FRAME_COUNT ) );
			for ( int i = 0; i < FRAME_TEST_ITERATIONS_PER_FRAME * FRAME_TEST_FRAME_COUNT; ++i)
			{
				allocationSizes[i] = rand() % UCHAR_MAX;
			}

			start = std::chrono::high_resolution_clock::now();
			TestFrameAllocator( allocationSizes );
			end = std::chrono::high_resolution_clock::now();

			duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
			LogOut << "Execution time of test: " << duration << " ms\n\n";
		}

		else if (input == "frametestThreaded" || input == "ft")
		{
			int threadCount;
			std::cout << "Input thread count\n";
			std::cin >> threadCount;

			srand( 40805 );
			unsigned char* allocationSizeArrays[THREAD_TEST_THREAD_COUNT_MAX];
			for ( int i = 0; i < threadCount; ++i )
			{
				allocationSizeArrays[i] = static_cast<unsigned char*>(malloc( sizeof( char ) * FRAME_TEST_ITERATIONS_PER_FRAME * FRAME_TEST_FRAME_COUNT ));
				for ( int j = 0; j < FRAME_TEST_ITERATIONS_PER_FRAME * FRAME_TEST_FRAME_COUNT; ++j )
				{
					allocationSizeArrays[i][j] = rand() % UCHAR_MAX;
				}
			}

			LogOut << "Starting frame allocator threaded test with "<< threadCount << " threads\n";
			std::chrono::steady_clock::time_point start, end;
			long long duration;
			
			start = std::chrono::high_resolution_clock::now();
			std::thread threads[THREAD_TEST_THREAD_COUNT_MAX];
			for ( int i = 0; i < threadCount; ++i )
			{
				threads[i] = std::thread( TestFrameAllocator, *allocationSizeArrays );
			}
			for ( int i = 0; i < threadCount; ++i )
			{
				threads[i].join();
			}
			end = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();

			LogOut << "Execution time of test: " << duration << " ms\n\n";
		}

		else if (input == "pooltest" || input == "p")
			TestPoolAllocator();

		else if (input == "pooltest2" || input == "p2")
			TestPoolAllocator2();

		else if (input == "help" || input == "?")
			PrintHelp();

		else
			LogOut << "Invalid command.\n\n";
	}
    return 0;
}

void TestFrameAllocator(unsigned char* mallocSizes)
{
	unsigned int mallocSizesIterator = 0;
	InitializeFrameAllocator( 32ULL * MEBI, 16ULL );

	for ( unsigned int i = 0; i < FRAME_TEST_FRAME_COUNT; ++i )
	{
		for ( unsigned int j = 0; j < FRAME_TEST_ITERATIONS_PER_FRAME; ++j )
		{
			Byte* memoryPointer	= static_cast<Byte*>( fMalloc( mallocSizes[mallocSizesIterator++] ) );
			fFree( memoryPointer );
		}
		ResetFrameAllocator();
	}

	ShutdownFrameAllocator();
}

void TestPoolAllocator()
{
	const unsigned int ALLOCATIONS = 100000;

	std::chrono::steady_clock::time_point start, end;
	long long duration;
	DebugStruct* debugStructArray[ALLOCATIONS];

	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		debugStructArray[i] = new DebugStruct(true, 5);
	}
	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		delete debugStructArray[i];
	}
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	LogOut << "Allocation test WITHOUT pool allocator: " << duration << " ms\n";

	start = std::chrono::high_resolution_clock::now();
	PoolAllocatorHandle handle = InitializePoolAllocator(sizeof( DebugStruct ), ALLOCATIONS, POOL_ALLOCATOR_DEFAULT_ALIGNMENT);

	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		debugStructArray[i] = pNew(handle, DebugStruct, true, 5);
	}
	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		pDelete(handle, debugStructArray[i]);
	}
	ShutdownPoolAllocator(handle);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	LogOut << "Allocation test WITH pool allocator: " << duration << " ms\n\n";
}

void TestPoolAllocator2()
{
	const unsigned int ALLOCATIONS = 100000;
	int notDone = ALLOCATIONS;

	std::chrono::steady_clock::time_point start, end;
	long long duration;
	std::vector<GameObject*> gameObjects;

	start = std::chrono::high_resolution_clock::now();
	PoolAllocatorHandle handle = InitializePoolAllocator(sizeof(DebugStruct), ALLOCATIONS, POOL_ALLOCATOR_DEFAULT_ALIGNMENT);

	srand(1337); // Set seed so the random is consistant

	while (notDone > 0 || gameObjects.size() > 0)
	{
		// Create new gameObjects
		while (rand() % 2 > 0)
		{
			notDone--;
			gameObjects.push_back(pNew(handle, GameObject, rand() % 20));
		}
		for (int i = static_cast<int>( gameObjects.size() )-1; i >= 0; --i)
		{
			if (gameObjects[i]->DeleteMe())
			{
				pDelete(handle, gameObjects[i]);
				gameObjects.erase(gameObjects.begin()+i);
			}
		}
	}

	ShutdownPoolAllocator(handle);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	LogOut << "Allocation test WITH pool allocator: " << duration << " ms\n\n";
}

void PrintHelp()
{
	LogOut << "The following commands are supported\n\n";

	LogOut << "- FrameTest (f)\n";
	LogOut << "- FrameThreadedTest (ft)\n";
	LogOut << "- PoolTest (p)\n";
	LogOut << "- PoolTest2 (p2)\n";
	LogOut << "- Quit (q)\n";

	LogOut << "\n";
}