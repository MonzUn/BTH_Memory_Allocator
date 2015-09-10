#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include "Logger.h"
#include <new>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>

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
	~DebugStruct() {
		int i = 0;
	}

	bool	Alpaca;
	int		NumberOfLegs;
};

void testFrameAllocator();
void testFrameAllocatorFill();
void testPoolAllocator();
void printHelp();


Logger logOut;

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
			testFrameAllocator();

		else if (input == "frametestfill")
			testFrameAllocatorFill();

		else if (input == "pooltest")
			testPoolAllocator();

		else if (input == "help" || input == "?")
			printHelp();

		else
			logOut << "Invalid command.\n\n";

	}
    return 0;
}

void testFrameAllocatorFill()
{
#ifdef DISABLE_FRAME_ALLOCATOR
	logOut << "Starting frame allocator test 'fill' without custom allocator.\n";
#else
	logOut << "Starting frame allocator test 'fill' with custom allocator.\n";
#endif

	FrameAllocator::Initialize();

	std::chrono::steady_clock::time_point start, end;
	long long duration;

	// Check if 64 or 32bit
#ifndef ENVIRONMENT64
	const unsigned int noObjects = 144631;
#else
	const unsigned int noObjects = 139810;
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
	
	logOut << "Execution time of test: " << duration << " ms\n\n";

	FrameAllocator::Reset();
	FrameAllocator::Shutdown();
}

void testFrameAllocator() 
{
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

	logOut << "\n";
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
	logOut << "Allocation test WITH pool allocator: " << duration << " ms\n\n";
}

void printHelp()
{
	logOut << "FrameTest\n";
	logOut << "FrameTestFill\n";
	logOut << "PoolTest\n";
	logOut << "Quit\n";

	logOut << "\n";
}