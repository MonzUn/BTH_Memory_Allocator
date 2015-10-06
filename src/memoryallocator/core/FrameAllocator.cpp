#include "FrameAllocator.h"

void FrameAllocator::Initialize( size_t memoryByteSize, size_t alignment )
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

void FrameAllocator::Shutdown()
{
	assert( mInitialized );
	free( mMemory );
	mInitialized = false;
}

void FrameAllocator::Reset()
{
	assert( mInitialized );
	mWalker = mMemory;
}