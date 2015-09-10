#pragma once
#include "FrameAllocator.h"

//#define DISABLE_FRAME_ALLOCATOR

#ifndef DISABLE_FRAME_ALLOCATOR
#define fMalloc( count ) MemoryAllocator::GetFrameAllocator()->Allocate<Byte>( count )
#define fNew( type, ... ) new( MemoryAllocator::GetFrameAllocator()->Allocate<type>( 1 ) ) type( __VA_ARGS__ )
#define fNewArray( type, count ) MemoryAllocator::GetFrameAllocator()->Create<type>( count )
#define fFree( pointer )
#define fDelete( pointer ) MemoryAllocator::GetFrameAllocator()->Destroy( pointer )
#define fDeleteArray( pointer ) MemoryAllocator::GetFrameAllocator()->Destroy ( pointer )
#else
#define fMalloc( count ) malloc( count )
#define fNew( type, ... ) new type( __VA_ARGS__ )
#define fNewArray( type, count ) new type[count]
#define fFree( pointer ) free( pointer )
#define fDelete( pointer ) delete pointer
#define fDeleteArray( pointer ) delete[] pointer
#endif

namespace MemoryAllocator
{
	static FrameAllocator FrameAlloc;

	FrameAllocator* GetFrameAllocator() { return &FrameAlloc; }
}