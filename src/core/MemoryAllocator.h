#pragma once
#include "FrameAllocator.h"

//#define DISABLE_FRAME_ALLOCATOR

#ifndef DISABLE_FRAME_ALLOCATOR
#define InitializeFrameAllocator( memoryByteSize, alignment ) MemoryAllocator::FrameAlloc.Initialize( memoryByteSize, alignment )
#define ShutdownFrameAllocator() MemoryAllocator::FrameAlloc.Shutdown()
#define ResetFrameAllocator() MemoryAllocator::FrameAlloc.Reset()

#define fMalloc( count ) MemoryAllocator::FrameAlloc.Allocate<Byte>( count )
#define fNew( type, ... ) new( MemoryAllocator::FrameAlloc.Allocate<type>( 1 ) ) type( __VA_ARGS__ )
#define fNewArray( type, count ) MemoryAllocator::FrameAlloc.Create<type>( count )
#define fFree( pointer )
#define fDelete( pointer ) MemoryAllocator::FrameAlloc.Destroy( pointer )
#define fDeleteArray( pointer ) MemoryAllocator::FrameAlloc.Destroy ( pointer )
#else
#define InitializeFrameAllocator
#define ShutdownFrameAllocator
#define ResetFrameAllocator

#define fMalloc( count ) malloc( count )
#define fNew( type, ... ) new type( __VA_ARGS__ )
#define fNewArray( type, count ) new type[count]
#define fFree( pointer ) free( pointer )
#define fDelete( pointer ) delete pointer
#define fDeleteArray( pointer ) delete[] pointer
#endif

namespace MemoryAllocator
{
	thread_local FrameAllocator FrameAlloc;
}