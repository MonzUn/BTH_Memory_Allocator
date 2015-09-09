#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include "Logger.h"
#include <new>
#include <iostream>
#include <string>
#include <chrono>

struct DebugStruct
{
	DebugStruct() {};
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};
	~DebugStruct() {
		int i = 0;
	}

	bool	Alpaca;
	int		NumberOfLegs;
};

void testFrameAllocator();
void testPoolAllocator();

Logger logOut;

int main()
{
	bool quit = false;
	while (!quit)
	{
		std::cout << "> ";
		std::string input;
		std::cin >> input;

		if (input == "quit")
			quit = true;

		if (input == "frameTest")
			testFrameAllocator();

		if (input == "poolTest")
			testPoolAllocator();
	}
    return 0;
}

void testFrameAllocator() {
	FrameAllocator::Initialize();

	unsigned int framesToRun = 64;
	do
	{
		for (unsigned int i = 0; i < 100000; ++i)
		{
			Byte*			memoryPointer		= static_cast<Byte*>( fMalloc( 100 ) );
			DebugStruct*	structPointer		= fNew( DebugStruct, true, 5 );
			DebugStruct*	structArrayPointer	= fNewArray( DebugStruct, 3 );

			fFree( memoryPointer );
			fDelete( structPointer );
			fDeleteArray( structArrayPointer );
		}

		FrameAllocator::Reset();
		--framesToRun;
	} while (framesToRun > 0);

	FrameAllocator::Shutdown();
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
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	logOut << "Allocation test WITHOUT pool allocator: " << duration << " ms\n";

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
	logOut << "Allocation test WITH pool allocator: " << duration << " ms\n";
}