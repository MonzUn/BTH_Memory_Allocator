#pragma once
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <mutex>
#include "AllocatorUtility.h"

#define FRAME_ALLOCATOR_DEBUG 1

#define FRAME_ALLOCATOR_DEFAULT_MEMORY_BYTE_SIZE 1 * MEBI
#define FRAME_ALLOCATOR_DEFAULT_ALIGNMENT 8
class FrameAllocator
{
public:
	void Initialize( size_t memoryByteSize = FRAME_ALLOCATOR_DEFAULT_MEMORY_BYTE_SIZE, size_t alignment = FRAME_ALLOCATOR_DEFAULT_ALIGNMENT )
	{
		assert( !mInitialized );

		assert( alignment != 0 );
		assert( ( alignment & ( ~alignment + 1 ) ) == alignment );	// The alignment must be a power of 2

		assert( memoryByteSize >= alignment );						// The memory size must be at least one alignment big
		assert( memoryByteSize % alignment == 0 );					// The memory size must be a multiple of the alignment

		mMemoryByteSize	= memoryByteSize;
		mAlignment		= alignment;

		mMemory = static_cast<Byte*>( malloc( memoryByteSize ) );
		mWalker = mMemory;

		mInitialized = true;
	}

	void Shutdown()
	{
		assert( mInitialized );
		free( mMemory );
		mInitialized = false;
	}

	void Reset()
	{
		assert( mInitialized );
		mWalker = mMemory;
	}

	template<typename T>
	T* Allocate( size_t count = 1ULL)
	{
		assert( mInitialized );
#if FRAME_ALLOCATOR_DEBUG == 1
		// Ensure that we don't run out of memory
		assert( mWalker + sizeof( T ) < mMemory + mMemoryByteSize );
#endif
		Byte* returnPos = mWalker;

		size_t size = count * sizeof( T ) + sizeof( size_t );
		size += ( size & mAlignment ) ? mAlignment - ( size & mAlignment ) : 0; // Align the memory
		mWalker += size;

		return reinterpret_cast<T*>( returnPos );
	}

	template<typename T>
	void Destroy( T*& pointer )
	{
		assert( mInitialized );

		if ( pointer != nullptr )
		{
			pointer->~T();
		}
	}

	template<typename T>
	T* SharedAllocate( size_t count = 1ULL ) 
	{
		assert( mInitialized );
		mLock.lock();
		T* toReturn = Allocate<T>( count );
		mLock.unlock();
		return toReturn;
	}

private:
	Byte*	mMemory;
	Byte*	mWalker;

	size_t	mMemoryByteSize;
	size_t	mAlignment;

	bool	mInitialized = false;

	std::mutex mLock;
};