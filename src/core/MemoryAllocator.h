#pragma once
#include <thread>
#include <mutex>
#include <math.h>
#include "FrameAllocator.h"
#include "PoolAllocator.h"

//#define DISABLE_CUSTOM_ALLOCATORS
//#define DISABLE_FRAME_ALLOCATOR
//#define DISABLE_POOL_ALLOCATOR

#define POOL_ALLOCATOR_MAX_COUNT 127
#define INVALID_POOL_ALLOCATOR_HANDLE -1
typedef char PoolAllocatorHandle;

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

	// Note that Initialize, Shutdown and Reset must be called in a state where no other shared frame allocator functions may be called at the same time
	#define InitializeSharedFrameAllocator( memoryByteSize, alignment ) MemoryAllocator::SharedFrameAllocator.Initialize( memoryByteSize, alignment )
	#define ShutdownSharedFrameAllocator() MemoryAllocator::SharedFrameAllocator.Shutdown()
	#define ResetSharedFrameAllocator() MemoryAllocator::SharedFrameAllocator.Reset()
	
	#define fSharedMalloc( count ) MemoryAllocator::SharedFrameAllocator.SharedAllocate<Byte>( count )
	#define fSharedNew( type, ... ) new( MemoryAllocator::SharedFrameAllocator.SharedAllocate<type>( 1 ) ) type( __VA_ARGS__ )
	#define fSharedNewArray( type, count ) MemoryAllocator::SharedFrameAllocator.SharedCreate<type>( count )
	#define fSharedFree( pointer )
	#define fSharedDelete( pointer ) MemoryAllocator::SharedFrameAllocator.Destroy( pointer )
	#define fSharedDeleteArray( pointer ) MemoryAllocator::SharedFrameAllocator.Destroy ( pointer )
#else
	#define InitializeFrameAllocator( dummy1, dummy2 )
	#define ShutdownFrameAllocator()
	#define ResetFrameAllocator()

	#define fMalloc( count ) malloc( count )
	#define fNew( type, ... ) new type( __VA_ARGS__ )
	#define fNewArray( type, count ) new type[count]
	#define fFree( pointer ) free( pointer )
	#define fDelete( pointer ) delete pointer
	#define fDeleteArray( pointer ) delete[] pointer

	#define InitializeSharedFrameAllocator()
	#define ShutdownSharedFrameAllocator()
	#define ResetSharedFrameAllocator()

	#define fSharedMalloc( count ) malloc( count )
	#define fSharedNew( type, ... ) new type( __VA_ARGS__ )
	#define fSharedNewArray( type, count ) new type[count]
	#define fSharedFree( pointer ) free( pointer )
	#define fSharedDelete( pointer ) delete pointer
	#define fSharedDeleteArray( pointer ) delete[] pointer
#endif

#ifndef DISABLE_POOL_ALLOCATOR
	#define InitializePoolAllocator( blockSize, blockCount, alignment ) MemoryAllocator::CreatePoolAllocator( blockSize, blockCount, alignment )
	#define ShutdownPoolAllocator( handle ) MemoryAllocator::RemovePoolAllocator( handle )
	#define pMalloc( handle, count ) MemoryAllocator::PoolAllocators[handle]->Allocate<Byte>( count )
	#define pNew( handle, type, ... ) new( MemoryAllocator::PoolAllocators[handle]->Allocate<type>() ) type( __VA_ARGS__ )
	#define pFree( handle, pointer ) MemoryAllocator::PoolAllocators[handle]->Deallocate( pointer ) 
	#define pDelete( handle, pointer ) MemoryAllocator::PoolAllocators[handle]->Deallocate( pointer ) // TODO: Call Destroy when pool allocator supports it

	#define InitializeSharedPoolAllocator( blockSize, blockCount, alignment ) MemoryAllocator::CreateSharedPoolAllocator( blockSize, blockCount, alignment )
	#define ShutdownSharedPoolAllocator( handle ) MemoryAllocator::RemoveSharedPoolAllocator( handle )
	#define pSharedMalloc( handle, count ) MemoryAllocator::SharedPoolAllocatorsLock.lock(); MemoryAllocator::SharedPoolAllocators[handle]->SharedAllocate<Byte>( count ); MemoryAllocator::SharedPoolAllocatorsLock.unlock()
	#define pSharedNew( handle, type, ... ) MemoryAllocator::SharedPoolAllocatorsLock.lock(); new( MemoryAllocator::SharedPoolAllocators[handle]->SharedAllocate<type>() ) type( __VA_ARGS__ ); MemoryAllocator::SharedPoolAllocatorsLock.unlock()
	#define pSharedFree( handle, pointer ) MemoryAllocator::SharedPoolAllocatorsLock.lock(); MemoryAllocator::SharedPoolAllocators[handle]->SharedDeallocate( pointer ); MemoryAllocator::SharedPoolAllocatorsLock.unlock()
	#define pSharedDelete( handle, pointer ) MemoryAllocator::SharedPoolAllocatorsLock.lock(); MemoryAllocator::PoolAllocators[handle]->SharedDeallocate( pointer ); MemoryAllocator::SharedPoolAllocatorsLock.lock() // TODO: Call Destroy when pool allocator supports it
#else
	#define InitializePoolAllocator( dummy1, dummy2, dummy3 )
	#define ShutdownPoolAllocator( dummy )

	#define pMalloc( count ) malloc( count )
	#define pNew( type, ... ) new type( __VA_ARGS__ )
	#define pFree( pointer ) free( pointer )
	#define pDelete( pointer ) delete pointer

	#define InitializeSharedPoolAllocator( dummy1, dummy2, dummy3 )
	#define ShutdownSharedPoolAllocator( dummy )
	
	#define pSharedMalloc( count ) malloc( count )
	#define pSharedNew( type, ... ) new type( __VA_ARGS__ )
	#define pSharedFree( pointer ) free( pointer )
	#define pSharedDelete( pointer ) delete pointer
#endif

namespace MemoryAllocator // Will be hidden by DLL interface
{
	thread_local FrameAllocator FrameAlloc;
	thread_local PoolAllocator* PoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };

	FrameAllocator				SharedFrameAllocator;
	PoolAllocator*				SharedPoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };
	std::mutex					SharedPoolAllocatorsLock;

	PoolAllocatorHandle CreatePoolAllocator( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
		PoolAllocatorHandle handle = INVALID_POOL_ALLOCATOR_HANDLE;
		for ( char i = 0; i < POOL_ALLOCATOR_MAX_COUNT; ++i )
		{
			if ( PoolAllocators[i] == nullptr )
			{
				handle = i;
				break;
			}
		}
		assert( handle != INVALID_POOL_ALLOCATOR_HANDLE ); // Assert that POOL_ALLOCATOR_MAX_COUNT has not been reached

		PoolAllocator* poolAllocator = new PoolAllocator();
		poolAllocator->Initialize( blockSize, blockCount, alignment );

		PoolAllocators[handle] = poolAllocator;

		return handle;
	}

	PoolAllocatorHandle CreateSharedPoolAllocator( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
		PoolAllocatorHandle handle = INVALID_POOL_ALLOCATOR_HANDLE;
		SharedPoolAllocatorsLock.lock();
		for ( char i = 0; i < POOL_ALLOCATOR_MAX_COUNT; ++i )
		{
			if ( PoolAllocators[i] == nullptr )
			{
				handle = i;
				break;
			}
		}
		assert( handle != INVALID_POOL_ALLOCATOR_HANDLE ); // Assert that POOL_ALLOCATOR_MAX_COUNT has not been reached

		PoolAllocator* poolAllocator = new PoolAllocator();
		poolAllocator->Initialize( blockSize, blockCount, alignment );

		SharedPoolAllocators[handle] = poolAllocator;
		SharedPoolAllocatorsLock.unlock();

		return handle;
	}

	void RemovePoolAllocator( PoolAllocatorHandle handle )
	{
		assert( PoolAllocators[handle] != nullptr ); // Assert that the allocator to be shut down exists

		PoolAllocators[handle]->Shutdown();
		delete PoolAllocators[handle];
		PoolAllocators[handle] = nullptr;
	}

	void RemoveSharedPoolAllocator( PoolAllocatorHandle handle )
	{
		SharedPoolAllocatorsLock.lock();
		assert( SharedPoolAllocators[handle] != nullptr ); // Assert that the allocator to be shut down exists

		SharedPoolAllocators[handle]->Shutdown();
		delete SharedPoolAllocators[handle];
		SharedPoolAllocators[handle] = nullptr;
		SharedPoolAllocatorsLock.unlock();
	}
}