#include "FrameAllocator.h"
#include <new>

#define TEST_FRAME_ALLOCATOR 1

struct DebugStruct
{
	DebugStruct( bool alpaca, int numberOfLegs ) : Alpaca( alpaca ), NumberOfLegs( numberOfLegs ) {};

	bool	Alpaca;
	int		NumberOfLegs;
};

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
    return 0;
}