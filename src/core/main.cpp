#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include <new>
#include <iostream>
#include <chrono>

#define TEST_FRAME_ALLOCATOR 1
#define TEST_POOL_ALLOCATOR 1

struct DebugStruct
{
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

void testPoolAllocator();

int main() 
{
#if TEST_FRAME_ALLOCATOR == 1
	FrameAllocator::Initialize();

	unsigned int framesToRun = 128;
	do
	{
		for ( unsigned int i = 0; i < 100000; ++i )
		{
			Byte*			memoryPointer	= static_cast<Byte*>( fMalloc( 100 ) );
			DebugStruct*	structPointer	= fNew( DebugStruct, true, 5 );

			fFree( memoryPointer );
			fDelete( structPointer );
		}

		FrameAllocator::Reset();
		--framesToRun;
	} while ( framesToRun > 0 );

	FrameAllocator::Shutdown();
#endif

#if TEST_POOL_ALLOCATOR == 1
	testPoolAllocator();
#endif

    return 0;
}

void testPoolAllocator()
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
	duration = std::chrono::duration_cast< std::chrono::milliseconds >(end - start).count();
	std::cout << "Allocation test WITHOUT pool allocator: " << duration << " ms" << std::endl;

	start = std::chrono::high_resolution_clock::now();
	PoolAllocator* poolAllocator = new PoolAllocator(sizeof(DebugStruct), ALLOCATIONS);
	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		debugStructArray[i] = new(poolAllocator->Allocate<DebugStruct>()) DebugStruct(true, 5);
	}
	for (size_t i = 0; i < ALLOCATIONS; ++i)
	{
		poolAllocator->Deallocate(debugStructArray[i]);
	}
	delete poolAllocator;
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast< std::chrono::milliseconds >(end - start).count();
	std::cout << "Allocation test WITH pool allocator: " << duration << " ms" << std::endl;
}