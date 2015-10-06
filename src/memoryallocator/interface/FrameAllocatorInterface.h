#pragma once
#include "../core/MemoryAllocatorInstanceManager.h"
#include "MemoryAllocatorControlDefines.h"

#ifndef DISABLE_FRAME_ALLOCATOR
#define InitializeFrameAllocator( memoryByteSize, alignment ) MemoryAllocatorInstanceManager::FrameAlloc.Initialize( memoryByteSize, alignment )
#define ShutdownFrameAllocator() MemoryAllocatorInstanceManager::FrameAlloc.Shutdown()
#define ResetFrameAllocator() MemoryAllocatorInstanceManager::FrameAlloc.Reset()

#define fMalloc( count ) MemoryAllocatorInstanceManager::FrameAlloc.Allocate<Byte>( count )
#define fNew( type, ... ) new( MemoryAllocatorInstanceManager::FrameAlloc.Allocate<type>( 1 ) ) type( __VA_ARGS__ )
#define fFree( pointer )
#define fDelete( pointer ) MemoryAllocatorInstanceManager::FrameAlloc.Destroy( pointer )

// Note that Initialize, Shutdown and Reset must be called in a state where no other shared frame allocator functions may be called at the same time
#define InitializeSharedFrameAllocator( memoryByteSize, alignment ) MemoryAllocatorInstanceManager::SharedFrameAllocator.Initialize( memoryByteSize, alignment )
#define ShutdownSharedFrameAllocator() MemoryAllocatorInstanceManager::SharedFrameAllocator.Shutdown()
#define ResetSharedFrameAllocator() MemoryAllocatorInstanceManager::SharedFrameAllocator.Reset()

#define fSharedMalloc( count ) MemoryAllocatorInstanceManager::SharedFrameAllocator.SharedAllocate<Byte>( count )
#define fSharedNew( type, ... ) new( MemoryAllocatorInstanceManager::SharedFrameAllocator.SharedAllocate<type>( 1 ) ) type( __VA_ARGS__ )
#define fSharedFree( pointer )
#define fSharedDelete( pointer ) MemoryAllocatorInstanceManager::SharedFrameAllocator.Destroy( pointer )
#else
#define InitializeFrameAllocator( memoryByteSize, alignment )
#define ShutdownFrameAllocator()
#define ResetFrameAllocator()

#define fMalloc( count ) malloc( count )
#define fNew( type, ... ) new type( __VA_ARGS__ )
#define fFree( pointer ) free( pointer )
#define fDelete( pointer ) delete pointer

#define InitializeSharedFrameAllocator( memoryByteSize, alignment )
#define ShutdownSharedFrameAllocator()
#define ResetSharedFrameAllocator()

#define fSharedMalloc( count ) malloc( count )
#define fSharedNew( type, ... ) new type( __VA_ARGS__ )
#define fSharedFree( pointer ) free( pointer )
#define fSharedDelete( pointer ) delete pointer
#endif