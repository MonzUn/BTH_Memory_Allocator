#pragma once
#include <thread>
#include <mutex>
#include <math.h>
#include "FrameAllocator.h"
#include "PoolAllocator.h"

//#define DISABLE_CUSTOM_ALLOCATORS
//#define DISABLE_FRAME_ALLOCATOR
//#define DISABLE_POOL_ALLOCATOR

#define POOL_ALLOCATOR_MAX_COUNT 30

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
	#define ShutdownPoolAllocator( blockSize ) MemoryAllocator::RemovePoolAllocator( blockSize )

	#define pMalloc( count ) MemoryAllocator::FindFittingPoolAllocator( count )->Allocate<Byte>( count )
	#define pNew( type, ... ) new( MemoryAllocator::FindFittingPoolAllocator( sizeof( type ) )->Allocate<type>() ) type( __VA_ARGS__ ) // TODO: Input 1 as parameter to Allocate when it supports arrays
	//#define pNewArray( type, count ) // TODO: Add when pool allocator supports arrays
	#define pFree( pointer ) MemoryAllocator::FindFittingPoolAllocator( *pointer )->Deallocate( pointer ) 
	#define pDelete( pointer ) MemoryAllocator::FindFittingPoolAllocator( sizeof( *pointer ) )->Deallocate( pointer ) // TODO: Call Destroy when pool allocator supports it
	//#define pDeleteArray( pointer ) // TODO: Add when pool allocator supports arrays

	#define InitializeSharedPoolAllocator( blockSize, blockCount, alignment ) MemoryAllocator::CreateSharedPoolAllocator( blockSize, blockCount, alignment )
	#define ShutdownSharedPoolAllocator( blockSize ) MemoryAllocator::RemoveSharedPoolAllocator( blockSize )

	//#define pSharedMalloc( count ) MemoryAllocator::FindFittingSharedPoolAllocator( count )->SharedAllocate<Byte>( count ) // TODO: Add when pool allocator supports thread safe allocate
	//#define pSharedNew( type, ... ) new( MemoryAllocator::FindFittingPoolAllocator( sizeof( type ) )->SharedAllocate<type>() ) type( __VA_ARGS__ ) // TODO: Add when pool allocator supports thread safe allocate // TODO: Input 1 as parameter to Allocate when it supports arrays
	//#define pSharedNewArray( type, count ) // TODO: Add when pool allocator supports arrays
	//#define pSharedFree( pointer ) MemoryAllocator::FindFittingSharedPoolAllocator( *pointer )->SharedDeallocate( pointer ) // TODO: Add when pool allocator supports shared deallocate
	//#define pSharedDelete( pointer ) MemoryAllocator::FindFittingSharedPoolAllocator( sizeof( *pointer ) )->Deallocate( pointer ) // TODO: Add when pool allocator supports shared deallocate // TODO: Call Destroy when pool allocator supports it
	//#define pSharedDeleteArray( pointer ) // TODO: Add when pool allocator supports arrays
#else
	#define InitializePoolAllocator( dummy1, dummy2, dummy3 )
	#define ShutDownPoolAllocator( dummy )

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
	thread_local PoolAllocator* PoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };

	FrameAllocator				SharedFrameAllocator;
	PoolAllocator*				SharedPoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };
	std::mutex					SharedPoolAllocatorsLock;

	void CreatePoolAllocator( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
		unsigned char index = static_cast<unsigned char>( log2( blockSize ) );
		assert( PoolAllocators[index] == nullptr ); // Assert that we aren't creating a duplicate

		PoolAllocator* poolAllocator = new PoolAllocator();
		poolAllocator->Initialize( blockSize, blockCount, alignment );

		PoolAllocators[index] = poolAllocator;
	}

	void CreateSharedPoolAllocator( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
		unsigned char index = static_cast<unsigned char>( log2( blockSize ) );

		SharedPoolAllocatorsLock.lock();
		assert( PoolAllocators[index] == nullptr ); // Assert that we aren't creating a duplicate

		PoolAllocator* poolAllocator = new PoolAllocator();
		poolAllocator->Initialize( blockSize, blockCount, alignment );

		PoolAllocators[index] = poolAllocator;
		SharedPoolAllocatorsLock.unlock();
	}

	void RemovePoolAllocator( size_t blockSize )
	{
		unsigned char index = static_cast<unsigned char>( log2( blockSize ) );
		assert( PoolAllocators[index] != nullptr ); // Assert that the allocator to be shut down exists

		PoolAllocators[index]->Shutdown();
		delete PoolAllocators[index];
		PoolAllocators[index] = nullptr;
	}

	void RemoveSharedPoolAllocator( size_t blockSize )
	{
		unsigned char index = static_cast<unsigned char>( log2( blockSize ) );

		SharedPoolAllocatorsLock.lock();
		assert( PoolAllocators[index] != nullptr ); // Assert that the allocator to be shut down exists

		SharedPoolAllocators[index]->Shutdown();
		delete SharedPoolAllocators[index];
		SharedPoolAllocators[index] = nullptr;
		SharedPoolAllocatorsLock.unlock();
	}

	PoolAllocator* FindFittingPoolAllocator( size_t byteSize )
	{
		PoolAllocator* returnAdress = nullptr;

		for ( unsigned int i = 0; i < POOL_ALLOCATOR_MAX_COUNT; ++i ) // We could calculate a lowest possible start index here but that would take more time than to loop from the beginning (Profiled)
		{
			if ( PoolAllocators[i] != nullptr )
			{
				returnAdress = PoolAllocators[i];
				break;
			}
		}

		assert( returnAdress != nullptr ); // There was no allocator with sufficiently big block size
		return returnAdress;
	}

	PoolAllocator* FindFittingSharedPoolAllocator( size_t byteSize )
	{
		PoolAllocator* returnAdress = nullptr;

		SharedPoolAllocatorsLock.lock();

		for ( unsigned int i = 0; i < POOL_ALLOCATOR_MAX_COUNT; ++i ) // We could calculate a lowest possible start index here but that would take more time than to loop from the beginning (Profiled)
		{
			if ( SharedPoolAllocators[i] != nullptr ) {
				returnAdress = SharedPoolAllocators[i];
				break;
			}
		}

		SharedPoolAllocatorsLock.lock();

		assert( returnAdress != nullptr ); // There was no allocator with sufficiently big block size
		return returnAdress;
	}
}