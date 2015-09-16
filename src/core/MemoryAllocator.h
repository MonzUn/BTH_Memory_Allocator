#pragma once
#include <map>
#include <thread>
#include "FrameAllocator.h"
#include "PoolAllocator.h"

//#define DISABLE_CUSTOM_ALLOCATORS
//#define DISABLE_FRAME_ALLOCATOR
//#define DISABLE_POOL_ALLOCATOR

//#define USE_SINGLE_POOL_ALLOCATOR // Undefine this to test the code used when there are multiple pool allocators for each thread

#ifdef DISABLE_CUSTOM_ALLOCATORS
#define DISABLE_FRAME_ALLOCATOR
#define DISABLE_POOL_ALLOCATOR
#endif

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
#define InitializeFrameAllocator()
#define ShutdownFrameAllocator()
#define ResetFrameAllocator()

#define fMalloc( count ) malloc( count )
#define fNew( type, ... ) new type( __VA_ARGS__ )
#define fNewArray( type, count ) new type[count]
#define fFree( pointer ) free( pointer )
#define fDelete( pointer ) delete pointer
#define fDeleteArray( pointer ) delete[] pointer
#endif

#ifndef DISABLE_POOL_ALLOCATOR
#ifdef USE_SINGLE_POOL_ALLOCATOR
#define InitializePoolAllocator( blockSize, blockCount, alignment ) MemoryAllocator::CreatePoolAllocator( blockSize, blockCount, alignment )
#define ShutDownPoolAllocator( blockSize ) MemoryAllocator::RemovePoolAllocator( blockSize )

#define pMalloc( count ) MemoryAllocator::PoolAlloc.Allocate<Byte>( count )
#define pNew( type, ... ) new( MemoryAllocator::PoolAlloc.Allocate<type>() ) type( __VA_ARGS__ ) // TODO: Input 1 as parameter to Allocate when it supports arrays
//#define pNewArray( type, count ) // TODO: Add when pool allocator supports arrays
#define pFree( pointer ) MemoryAllocator::PoolAlloc.Deallocate( pointer ) 
#define pDelete( pointer ) MemoryAllocator::PoolAlloc.Deallocate( pointer ) // TODO: Call Destroy when pool allocator supports it
//#define pDeleteArray( pointer ) // TODO: Add when pool allocator supports arrays
#else
#define InitializePoolAllocator( blockSize, blockCount, alignment ) MemoryAllocator::CreatePoolAllocator( blockSize, blockCount, alignment )
#define ShutDownPoolAllocator( blockSize ) MemoryAllocator::RemovePoolAllocator( blockSize )

#define pMalloc( count ) MemoryAllocator::FindFittingPoolAllocator( count )->Allocate<Byte>( count )
#define pNew( type, ... ) new( MemoryAllocator::FindFittingPoolAllocator( sizeof( type ) )->Allocate<type>() ) type( __VA_ARGS__ ) // TODO: Input 1 as parameter to Allocate when it supports arrays
//#define pNewArray( type, count ) // TODO: Add when pool allocator supports arrays
#define pFree( pointer ) MemoryAllocator::FindFittingPoolAllocator( *pointer )->Deallocate( pointer ) 
#define pDelete( pointer ) MemoryAllocator::FindFittingPoolAllocator( sizeof( *pointer ) )->Deallocate( pointer ) // TODO: Call Destroy when pool allocator supports it
//#define pDeleteArray( pointer ) // TODO: Add when pool allocator supports arrays
#endif

#else
#define InitializePoolAllocator()
#define ShutDownPoolAllocator()

#define pMalloc( count ) malloc( count )
#define pNew( type, ... ) new type( __VA_ARGS__ )
#define pNewArray( type, count ) new type[count]
#define pFree( pointer ) free( pointer )
#define pDelete( pointer ) delete pointer
#define pDeleteArray( pointer ) delete[] pointer
#endif

namespace MemoryAllocator // Will be hidden by DLL interface
{
	thread_local FrameAllocator FrameAlloc;
	thread_local std::map<size_t, PoolAllocator> PoolAllocators;
#ifdef USE_SINGLE_POOL_ALLOCATOR
	thread_local PoolAllocator PoolAlloc;
#endif

	void CreatePoolAllocator( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
#ifdef USE_SINGLE_POOL_ALLOCATOR
		PoolAlloc.Initialize( blockSize, blockCount, alignment );
#else
		assert( PoolAllocators.find( blockSize ) == PoolAllocators.end() ); // Assert that we aren't creating a duplicate

		PoolAllocator poolAllocator;
		poolAllocator.Initialize( blockSize, blockCount, alignment );

		PoolAllocators.emplace( blockSize, poolAllocator );
#endif
	}

	void RemovePoolAllocator( size_t blockSize )
	{
#ifdef USE_SINGLE_POOL_ALLOCATOR
		PoolAlloc.Shutdown();
#else
		assert( PoolAllocators.find( blockSize ) != PoolAllocators.end() ); // Assert that the allocator to be shut down exists

		PoolAllocators.at( blockSize ).Shutdown();
		PoolAllocators.erase( blockSize );
#endif
	}

#ifndef USE_SINGLE_POOL_ALLOCATOR
	PoolAllocator* FindFittingPoolAllocator( size_t byteSize )
	{
		PoolAllocator* returnAdress = nullptr;

		for ( auto& sizeAndAllocatorPair : PoolAllocators )
		{
			if( sizeAndAllocatorPair.first >= byteSize )
			{
				returnAdress = &sizeAndAllocatorPair.second;
				break;
			}
		}

		assert( returnAdress != nullptr ); // There was no allocator with sufficiently big block size
		return returnAdress;
	}
#endif
}