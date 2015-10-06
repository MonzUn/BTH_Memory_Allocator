#pragma once
#include <cstdlib>
#include <cassert>
#include <mutex>
#include "MemoryAllocatorLibraryDefine.h"

#define POOL_ALLOCATOR_DEFAULT_BLOCK_SIZE 
#define POOL_ALLOCATOR_DEFAULT_ALIGNMENT 0x8

class PoolAllocator
{
public:
	MEMORYALLOCATOR_API void Initialize( size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT );

	MEMORYALLOCATOR_API void Shutdown();

	MEMORYALLOCATOR_API void Deallocate( void* block );

	MEMORYALLOCATOR_API void* SharedAllocate();

	MEMORYALLOCATOR_API void SharedDeallocate( void* block );

	template <class T>
	T* Allocate()
	{
		// Make sure that the block size is bigger than or equal to T
		assert( mBlockSize >= sizeof( T ) );

		return reinterpret_cast<T*>( Allocate() );
	}

    template <class T>
    void Deallocate( T* block )
    {
        block->~T();
        Deallocate( reinterpret_cast<void*>( block ) );
    }

    template <class T>
    T* SharedAllocate()
    {
        // Make sure that the block size is bigger than or equal to T
        assert( mBlockSize >= sizeof( T ) );

        return reinterpret_cast<T*>( SharedAllocate() );
    }

    template <class T>
    void SharedDeallocate( T* block )
    {
        SharedDeallocate( reinterpret_cast<void*>( block ) );
    }

private:
	MEMORYALLOCATOR_API void* Allocate();

	size_t mBlockSize;
	size_t mBlockCount;

	void* mPool;
	uintptr_t* mFreeBlocks;

	bool mInitialized = false;

    std::mutex mLock;
};