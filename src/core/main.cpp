#include <new>
#include <chrono>
#include "MemoryAllocator.h"
#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include "Logger.h"

struct DebugStruct
{
	DebugStruct() {};
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

void TestFrameAllocator();
void TestPoolAllocator();

Logger LogOut;

int main()
{
	bool quit = false;
	while (!quit)
	{
		std::cout << "> ";
		std::string input;
		std::cin >> input;

		if ( input == "quit" )
			quit = true;

		if ( input == "frameTest" )
			TestFrameAllocator();

		if ( input == "poolTest" )
			TestPoolAllocator();
	}
    return 0;
}

void TestFrameAllocator() {
	MemoryAllocator::GetFrameAllocator()->Initialize( 32ULL * MEBI, 16ULL );

	const unsigned int framesToRun			= 64;
	const unsigned int iterationsPerFrame	= 100000;
	for ( unsigned int i = 0; i < framesToRun; ++i )
	{
		for ( unsigned int j = 0; j < iterationsPerFrame; ++j )
		{
			Byte*			memoryPointer		= static_cast<Byte*>( fMalloc( 100 ) );
			DebugStruct*	structPointer		= fNew( DebugStruct, true, 5 );
			DebugStruct*	structArrayPointer	= fNewArray( DebugStruct, 3 );

			fFree( memoryPointer );
			fDelete( structPointer );
			fDeleteArray( structArrayPointer );
		}
		MemoryAllocator::GetFrameAllocator()->Reset();
	}

	MemoryAllocator::GetFrameAllocator()->Shutdown();
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
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	LogOut << "Allocation test WITH pool allocator: " << duration << " ms\n";
}