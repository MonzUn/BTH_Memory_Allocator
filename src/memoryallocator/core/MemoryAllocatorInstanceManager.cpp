#include "MemoryAllocatorInstanceManager.h"

PoolAllocatorHandle MemoryAllocatorInstanceManager::CreatePoolAllocator( PoolAllocator** poolAllocators, size_t blockSize, size_t blockCount, size_t alignment )
{
	PoolAllocatorHandle handle = INVALID_POOL_ALLOCATOR_HANDLE;
	for ( char i = 0; i < POOL_ALLOCATOR_MAX_COUNT; ++i )
	{
		if ( poolAllocators[i] == nullptr )
		{
			handle = i;
			break;
		}
	}
	assert( handle != INVALID_POOL_ALLOCATOR_HANDLE ); // Assert that POOL_ALLOCATOR_MAX_COUNT has not been reached

	PoolAllocator* poolAllocator = new PoolAllocator();
	poolAllocator->Initialize( blockSize, blockCount, alignment );

	poolAllocators[handle] = poolAllocator;

	return handle;
}

void MemoryAllocatorInstanceManager::RemovePoolAllocator( PoolAllocator** poolAllocators, PoolAllocatorHandle& handle )
{
	assert( poolAllocators[handle] != nullptr ); // Assert that the allocator to be shut down exists

	poolAllocators[handle]->Shutdown();
	delete poolAllocators[handle];
	poolAllocators[handle] = nullptr;

	handle = INVALID_POOL_ALLOCATOR_HANDLE;
}