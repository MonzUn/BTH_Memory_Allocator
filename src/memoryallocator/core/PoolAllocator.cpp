#include "PoolAllocator.h"

void PoolAllocator::Initialize( size_t blockSize, size_t blockCount, size_t alignment )
{
	assert( !mInitialized );

	mBlockSize = blockSize;
	mBlockCount = blockCount;

	// Make sure that the block count is bigger than zero
	assert( mBlockCount > 0 );

	// Adjust block size after alignment
	if ( alignment < blockSize ) 
	{
		size_t misalignment = mBlockSize & alignment;
		if ( misalignment > 0 )
		{
			mBlockSize += alignment - misalignment;
		}
	}
	else if ( alignment > blockSize )
	{
		mBlockSize = alignment;
	}

	// Make sure that the block size is bigger than or equal to the size of a pointer
	assert( mBlockSize >= sizeof( uintptr_t ) );

	mPool = malloc( mBlockSize * mBlockCount );

	// Create a linked list with all free blocks
	uintptr_t poolAddress = reinterpret_cast<uintptr_t>( mPool );
	for ( size_t i = 0; i < mBlockCount; i++ ) {
		// Calculate block addresses
		uintptr_t currBlockAddress = poolAddress + i * mBlockSize;
		uintptr_t nextBlockAddress = i + 1 < mBlockCount ? currBlockAddress + mBlockSize : 0;
		// Put next block address in current block
		uintptr_t* currBlock = reinterpret_cast<uintptr_t*>( currBlockAddress );
		*currBlock = nextBlockAddress;
	}
	mFreeBlocks = reinterpret_cast<uintptr_t*>( poolAddress );

	mInitialized = true;
}

void PoolAllocator::Shutdown()
{
	assert( mInitialized );
	free( mPool );
	mInitialized = false;
}

void PoolAllocator::Deallocate( void* block )
{
	assert( mInitialized );

    // Put deallocated block address in the free blocks
    uintptr_t blockAddress = mFreeBlocks != nullptr ? reinterpret_cast<uintptr_t>( mFreeBlocks ) : 0;
    mFreeBlocks = reinterpret_cast<uintptr_t*>( block );
    *mFreeBlocks = blockAddress;
}

void* PoolAllocator::SharedAllocate()
{
	mLock.lock();
	void* block = Allocate();
	mLock.unlock();
	return block;
}

void PoolAllocator::SharedDeallocate( void* block )
{
	mLock.lock();
	Deallocate( block );
	mLock.unlock();
}

void* PoolAllocator::Allocate() {
	assert( mInitialized );

	// Make sure that we are not out of blocks
	assert( mFreeBlocks != nullptr );

	void* block = mFreeBlocks;

	// Forward the free blocks first pointer
	uintptr_t nextAddress = *mFreeBlocks;
	mFreeBlocks = nextAddress != 0 ? reinterpret_cast<uintptr_t*>( nextAddress ) : nullptr;

	return block;
}