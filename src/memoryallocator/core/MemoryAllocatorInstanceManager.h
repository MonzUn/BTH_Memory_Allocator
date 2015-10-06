#pragma once
#include <mutex>
#include "FrameAllocator.h"
#include "PoolAllocator.h"
#include "MemoryAllocatorLibraryDefine.h"

#define POOL_ALLOCATOR_MAX_COUNT 127
#define INVALID_POOL_ALLOCATOR_HANDLE -1
typedef char PoolAllocatorHandle;

namespace MemoryAllocatorInstanceManager
{
	thread_local FrameAllocator FrameAlloc;
	thread_local PoolAllocator* PoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };

	FrameAllocator				SharedFrameAllocator;
	PoolAllocator*				SharedPoolAllocators[POOL_ALLOCATOR_MAX_COUNT] = { nullptr };
	std::mutex					SharedPoolAllocatorsLock;

	MEMORYALLOCATOR_API PoolAllocatorHandle CreatePoolAllocator( PoolAllocator** poolAllocators, size_t blockSize, size_t blockCount, size_t alignment = POOL_ALLOCATOR_DEFAULT_ALIGNMENT );

	MEMORYALLOCATOR_API void RemovePoolAllocator( PoolAllocator** poolAllocators, PoolAllocatorHandle& handle );
}