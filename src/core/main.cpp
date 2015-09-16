#include <new>
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

struct DebugStruct
{
	DebugStruct() {};
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

void TestFrameAllocator();
void TestFrameAllocatorFill();
void TestPoolAllocator();
void PrintHelp();

Logger LogOut;

int main()
{
	bool quit = false;
	while (!quit)
	{
		std::cout << "> ";
		std::string input;
		std::cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);

		if (input == "quit")
			quit = true;

		else if (input == "frametest")
			TestFrameAllocator();

		else if (input == "frametestfill")
			TestFrameAllocatorFill();

		else if (input == "pooltest")
			TestPoolAllocator();

		else if (input == "help" || input == "?")
			PrintHelp();

		else
			LogOut << "Invalid command.\n\n";
	}
    return 0;
}

void TestFrameAllocator()
{
	MemoryAllocator::FrameAlloc.Initialize( 32ULL * MEBI, 16ULL );

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
		MemoryAllocator::FrameAlloc.Reset();
	}

	MemoryAllocator::FrameAlloc.Shutdown();

	LogOut << "\n";
}

void TestFrameAllocatorFill()
{
	LogOut << "Starting frame allocator test 'fill' with custom allocator.\n";

	MemoryAllocator::FrameAlloc.Initialize(32ULL * MEBI, 16ULL);

	std::chrono::steady_clock::time_point start, end;
	long long duration;

	// Check if 64 or 32bit
	// 32ULL * MEBI = (X * sizeof(T) + sizeof(size_t)) + (X * (100 * sizeof(T) + sizeof(size_t))
#ifndef ENVIRONMENT64
	const unsigned int noObjects = 310690;
#else
	const unsigned int noObjects = 289263;
#endif

	start = std::chrono::high_resolution_clock::now();
	Byte** test = fNewArray(Byte*, noObjects);

	for (unsigned int i = 0; i < noObjects; ++i)
		test[i] = static_cast<Byte*>(fMalloc(100));

	for (int i = noObjects - 1; i >= 0; --i)
		fFree(test[i]);

	fDeleteArray(test);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	MemoryAllocator::FrameAlloc.Reset();
	MemoryAllocator::FrameAlloc.Shutdown();

	LogOut << "Execution time of test: " << duration << " ms\n\n";
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
	LogOut << "Allocation test WITH pool allocator: " << duration << " ms\n\n";
}

void PrintHelp()
{
	LogOut << "FrameTest\n";
	LogOut << "FrameTestFill\n";
	LogOut << "PoolTest\n";
	LogOut << "Quit\n";

	LogOut << "\n";
}