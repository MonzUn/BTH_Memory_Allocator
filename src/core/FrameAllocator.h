#pragma once
#include <cstdlib>
#include <cassert>
#include <cstring>
#include "AllocatorUtility.h"

#define FRAME_ALLOCATOR_DEBUG 1

class FrameAllocator
{
public:
	void Initialize( size_t memoryByteSize = 1 * MEBI, size_t alignment = 16 )
	{
		assert( !mInitialized );

		assert( alignment != 0 );
		assert( ( alignment & ( ~alignment + 1 ) ) == alignment );	// The alignment must be a power of 2

		assert( memoryByteSize >= alignment );						// The memory size must be at least one alignment big
		assert( memoryByteSize % alignment == 0 );					// The memory size must be a multiple of the alignment

		mMemoryByteSize	= memoryByteSize;
		mAlignment			= alignment;

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

private:
	Byte*	mMemory;
	Byte*	mWalker;

	size_t	mMemoryByteSize;
	size_t	mAlignment;

	bool	mInitialized = false;
};