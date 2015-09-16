#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <cstdlib>
#include <cassert>

class PoolAllocator
{
public:
	void Initialize( size_t blockSize, size_t blockCount, size_t alignment = 0x8 )
	{
		assert( !m_Initialized );

		mBlockSize = blockSize;
		mBlockCount = blockCount;

		// Make sure that the block count is bigger than zero
		assert( mBlockCount > 0 );

		// Adjust block size after alignment
		if ( alignment < blockSize ) {
			size_t misalignment = mBlockSize & alignment;
			if ( misalignment > 0 ) {
				mBlockSize += alignment - misalignment;
			}
		} else if ( alignment > blockSize ) {
			mBlockSize = alignment;
		}

		// Make sure that the block size is bigger than or equal to the size of a pointer
		assert( mBlockSize >= sizeof( uintptr_t ) );

		mPool = malloc( mBlockSize * mBlockCount );

		// Create a linked list with all free blocks
		uintptr_t poolAddress = reinterpret_cast< uintptr_t >( mPool );
		for ( size_t i = 0; i < mBlockCount; i++ ) {
			// Calculate block addresses
			uintptr_t currBlockAddress = poolAddress + i * mBlockSize;
			uintptr_t nextBlockAddress = i + 1 < mBlockCount ? currBlockAddress + mBlockSize : 0;
			// Put next block address in current block
			uintptr_t* currBlock = reinterpret_cast< uintptr_t* >( currBlockAddress );
			*currBlock = nextBlockAddress;
		}
		mFreeBlocksFirst = reinterpret_cast< uintptr_t* >( poolAddress );
		mFreeBlocksLast = reinterpret_cast< uintptr_t* >( poolAddress + ( mBlockCount - 1 ) * mBlockSize );

		m_Initialized = true;
	}

	void Shutdown()
	{
		assert( m_Initialized );
		free( mPool );
		m_Initialized = false;
	}

	void* Allocate()
	{
		assert( m_Initialized );

		// Make sure that we are not out of blocks
		assert( mFreeBlocksFirst != nullptr );

		void* block = mFreeBlocksFirst;

		// Forward the free blocks first pointer
		uintptr_t nextAddress = *mFreeBlocksFirst;
		if ( nextAddress != 0 )
		{
			mFreeBlocksFirst = reinterpret_cast< uintptr_t* >(nextAddress);
		}
		else
		{
			mFreeBlocksFirst = nullptr;
			mFreeBlocksLast = nullptr;
		}

		return block;
	}

	template <class T>
	T* Allocate()
	{
		assert( m_Initialized );

		// Make sure that the block size is bigger than or equal to T
		assert( mBlockSize >= sizeof(T) );

		return reinterpret_cast< T* >(Allocate());
	}

	void Deallocate( void* block )
	{
		assert( m_Initialized );

		if ( mFreeBlocksLast != nullptr )
		{
			// Put deallocated block address in previous last block
			uintptr_t blockAddress = reinterpret_cast< uintptr_t >(block);
			*mFreeBlocksLast = blockAddress;
		}

		// Set free blocks last pointer to deallocated block
		mFreeBlocksLast = reinterpret_cast< uintptr_t* >( block );

		if ( mFreeBlocksFirst == nullptr )
		{
			mFreeBlocksFirst = mFreeBlocksLast;
		}
	}

private:
	size_t mBlockSize;
	size_t mBlockCount;

	void* mPool;

	uintptr_t* mFreeBlocksFirst;
	uintptr_t* mFreeBlocksLast;

	bool m_Initialized = false;
};

#endif