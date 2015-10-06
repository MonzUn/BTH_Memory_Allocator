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

	template <class T>
	T* Allocate()
	{
		// Make sure that the block size is bigger than or equal to T
		assert( mBlockSize >= sizeof( T ) );

		return reinterpret_cast<T*>( Allocate() );
	}

	void Deallocate( void* block )
	{
		assert( mInitialized );
        // Put deallocated block address in the free blocks
        uintptr_t blockAddress = mFreeBlocks != nullptr ? reinterpret_cast<uintptr_t>( mFreeBlocks ) : 0;
        mFreeBlocks = reinterpret_cast<uintptr_t*>( block );
        *mFreeBlocks = blockAddress;
	}

    template <class T>
    void Deallocate( T* block )
    {
        block->~T();
        Deallocate( reinterpret_cast<void*>( block ) );
    }

    void* SharedAllocate()
    {
        mLock.lock();
        void* block = Allocate();
        mLock.unlock();
        return block;
    }

    template <class T>
    T* SharedAllocate()
    {
        // Make sure that the block size is bigger than or equal to T
        assert( mBlockSize >= sizeof( T ) );

        return reinterpret_cast<T*>( SharedAllocate() );
    }

    void SharedDeallocate( void* block )
    {
        mLock.lock();
        Deallocate( block );
        mLock.unlock();
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