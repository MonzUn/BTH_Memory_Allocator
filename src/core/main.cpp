#include <new>
#include <vector>
#include <chrono>
#include <algorithm>
#include <thread>
#include <memoryallocator/interface/FrameAllocatorInterface.h>
#include <memoryallocator/interface/PoolAllocatorInterface.h>
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
#define FRAME_TEST_FRAME_COUNT 128
#define FRAME_TEST_ITERATIONS_PER_FRAME 100000

#define POOL_TEST_ALLOCATIONS 10000000
#define POOL_TEST_BLOCK_SIZE 8

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

	int	LifeTime;
};

void TestFrameAllocator(unsigned char* mallocSizes, unsigned long long iterationsPerFrame);
void TestPoolAllocator( const uint32_t* order, size_t count );
void TestPoolAllocator2();
void PrintHelp();

int Random( int i ) { return std::rand() % i; }

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
			TestFrameAllocator( allocationSizes, FRAME_TEST_ITERATIONS_PER_FRAME );
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
				threads[i] = std::thread( TestFrameAllocator, *allocationSizeArrays, FRAME_TEST_ITERATIONS_PER_FRAME / threadCount );
			}
			for ( int i = 0; i < threadCount; ++i )
			{
				threads[i].join();
			}
			end = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();

			LogOut << "Execution time of test: " << duration << " ms\n\n";
		}

        else if ( input == "pooltest" || input == "p" )
        {
            srand( 40805 );
            std::vector<uint32_t> order( POOL_TEST_ALLOCATIONS * 3 );
            for ( uint32_t i = 0; i < POOL_TEST_ALLOCATIONS; ++i )
            {
                order[ i * 3 + 0 ] = i;
                order[ i * 3 + 1 ] = i;
                order[ i * 3 + 2 ] = i;
            }
            std::random_shuffle( order.begin(), order.end(), Random );

            std::chrono::steady_clock::time_point start, end;
            start = std::chrono::high_resolution_clock::now();

            TestPoolAllocator( &order[ 0 ], POOL_TEST_ALLOCATIONS );
            
            end = std::chrono::high_resolution_clock::now();
            long long duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
            LogOut << "Execution time of test: " << duration << " ms\n\n";
        }

        else if ( input == "pooltestThreaded" || input == "pt" )
        {
            int threadCount;
            std::cout << "Input thread count\n";
            std::cin >> threadCount;

            srand( 40805 );
            std::vector<std::vector<uint32_t>> orders( threadCount );
            for ( int t = 0; t < threadCount; ++t )
            {
                orders[ t ].resize( POOL_TEST_ALLOCATIONS * 3 );
                for ( uint32_t i = 0; i < POOL_TEST_ALLOCATIONS; ++i )
                {
                    orders[ t ][ i * 3 + 0 ] = i;
                    orders[ t ][ i * 3 + 1 ] = i;
                    orders[ t ][ i * 3 + 2 ] = i;
                }
                std::random_shuffle( orders[ t ].begin(), orders[ t ].end(), Random );
            }

            std::thread threads[ THREAD_TEST_THREAD_COUNT_MAX ];

            std::chrono::steady_clock::time_point start, end;
            start = std::chrono::high_resolution_clock::now();

            for ( int t = 0; t < threadCount; ++t )
            {
                threads[ t ] = std::thread( TestPoolAllocator, &orders[ t ][ 0 ], POOL_TEST_ALLOCATIONS );
            }
            for ( int t = 0; t < threadCount; ++t )
            {
                threads[ t ].join();
            }

            end = std::chrono::high_resolution_clock::now();
            long long duration = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
            LogOut << "Execution time of test: " << duration << " ms\n\n";
        }

		else if (input == "pooltest2" || input == "p2")
			TestPoolAllocator2();

		else if (input == "help" || input == "?")
			PrintHelp();

		else
			LogOut << "Invalid command.\n\n";
	}
    return 0;
}

void TestFrameAllocator(unsigned char* mallocSizes, unsigned long long iterationsPerFrame)
{
	unsigned int mallocSizesIterator = 0;
	InitializeFrameAllocator( 32ULL * MEBI, 16ULL );
	Byte** currentIterationAllocations = static_cast<Byte**>( malloc( sizeof( Byte* ) * iterationsPerFrame ) );

	for ( unsigned int i = 0; i < FRAME_TEST_FRAME_COUNT; ++i )
	{
		for ( unsigned int j = 0; j < iterationsPerFrame; ++j )
		{
			currentIterationAllocations[j] = static_cast<Byte*>( fMalloc( mallocSizes[mallocSizesIterator++] ) );
		}

		for ( int j = 0; j < iterationsPerFrame; ++j )
		{
			fFree( currentIterationAllocations[j] );
		}

		ResetFrameAllocator();
	}
	
	free( currentIterationAllocations );
	ShutdownFrameAllocator();
}

void TestPoolAllocator( const uint32_t* order, size_t count )
{
    std::vector<DebugStruct*> allocations( count, NULL );

	PoolAllocatorHandle handle = InitializePoolAllocator( sizeof(DebugStruct), count, POOL_ALLOCATOR_DEFAULT_ALIGNMENT );
    for (size_t i = 0; i < count * 3; ++i)
	{
        // Allocation
        if ( allocations[ order[ i ] ] == NULL )
            allocations[ order[ i ] ] = pNew( handle, DebugStruct, true, 4 );
        // Assignment
        else if ( allocations[ order[ i ] ]->Alpaca )
            allocations[ order[ i ] ]->Alpaca = false;
        // Deallocation
        else
            pDelete( handle, allocations[ order[ i ] ] );
	}
	ShutdownPoolAllocator( handle );
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
    LogOut << "- PoolTestThreaded (pt)\n";
	LogOut << "- PoolTest2 (p2)\n";
	LogOut << "- Quit (q)\n";

	LogOut << "\n";
}