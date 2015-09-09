#include "FrameAllocator.h"
#include <new>
#include <iostream>
#include <string>

#define TEST_FRAME_ALLOCATOR 1

struct DebugStruct
{
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

void DanielsTest() {
	FrameAllocator::Initialize();

	unsigned int framesToRun = 128;
	do
	{
		for (unsigned int i = 0; i < 100000; ++i)
		{
			Byte*			memoryPointer = static_cast<Byte*>(fMalloc(100));
			DebugStruct*	structPointer = fNew(DebugStruct, true, 5);

			fFree(memoryPointer);
			fDelete(structPointer);
		}

		FrameAllocator::Reset();
		--framesToRun;
	} while (framesToRun > 0);

	FrameAllocator::Shutdown();
}


int main() 
{
	bool quit = false;
	while (!quit)
	{
		std::string input;
		std::cin >> input;

		if (input == "daniel")
			DanielsTest();

	}
    return 0;
}