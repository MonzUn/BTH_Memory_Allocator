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
		memcpy( mWalker, &count, sizeof( size_t ) );

		Byte* returnPos = mWalker + sizeof( size_t );

		size_t size = count * sizeof( T ) + sizeof( size_t );
		size += ( size & mAlignment ) ? mAlignment - ( size & mAlignment ) : 0; // Align the memory
		mWalker += size;

		return reinterpret_cast<T*>( returnPos );
	}

	template<typename T>
	T* Create( size_t count )
	{
		assert( mInitialized );
		T* pointer = Allocate<T>( count );
		for ( size_t i = 0; i < count; ++i )
		{
			new( &pointer[i] ) T();
		}

		return pointer;
	}

	template<typename T>
	void Destroy( T*& pointer )
	{
		assert( mInitialized );
		size_t count = 0;
		memcpy( &count, reinterpret_cast<Byte*>( pointer ) - sizeof( size_t ), sizeof( size_t ) );

		if ( pointer != nullptr )
		{
			for ( size_t i = 0; i < count; ++i )
			{
				pointer[i].~T();
			}
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

	template<typename T>
	T* SharedCreate( size_t count )
	{
		assert( mInitialized );
		T* pointer = SharedAllocate<T>( count );
		for ( size_t i = 0; i < count; ++i ) {
			new( &pointer[i] ) T();
		}

		return pointer;
	}

private:
	Byte*	mMemory;
	Byte*	mWalker;

	size_t	mMemoryByteSize;
	size_t	mAlignment;

	bool	mInitialized = false;

	std::mutex mLock;
};