#pragma once
#include "../core/MemoryAllocatorInstanceManager.h"
#include "MemoryAllocatorControlDefines.h"

#ifndef DISABLE_POOL_ALLOCATOR
#define InitializePoolAllocator( blockSize, blockCount, alignment ) MemoryAllocatorInstanceManager::CreatePoolAllocator( MemoryAllocatorInstanceManager::PoolAllocators, blockSize, blockCount, alignment )
#define ShutdownPoolAllocator( handle ) MemoryAllocatorInstanceManager::RemovePoolAllocator( MemoryAllocatorInstanceManager::PoolAllocators, handle )

#define pMalloc( handle, count ) MemoryAllocatorInstanceManager::PoolAllocators[handle]->Allocate<Byte>()
#define pNew( handle, type, ... ) new( MemoryAllocatorInstanceManager::PoolAllocators[handle]->Allocate<type>() ) type( __VA_ARGS__ )
#define pFree( handle, pointer ) MemoryAllocatorInstanceManager::PoolAllocators[handle]->Deallocate( pointer ) 
#define pDelete( handle, pointer ) MemoryAllocatorInstanceManager::PoolAllocators[handle]->Deallocate( pointer )

#define InitializeSharedPoolAllocator( blockSize, blockCount, alignment ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); MemoryAllocatorInstanceManager::CreateSharedPoolAllocator( MemoryAllocatorInstanceManager::SharedPoolAllocators, blockSize, blockCount, alignment ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock()
#define ShutdownSharedPoolAllocator( handle ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); MemoryAllocatorInstanceManager::RemoveSharedPoolAllocator( MemoryAllocator::SharedPoolAllocators, handle ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock();

#define pSharedMalloc( handle, count ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); MemoryAllocatorInstanceManager::SharedPoolAllocators[handle]->SharedAllocate<Byte>( ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock()
#define pSharedNew( handle, type, ... ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); new( MemoryAllocatorInstanceManager::SharedPoolAllocators[handle]->SharedAllocate<type>() ) type( __VA_ARGS__ ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock()
#define pSharedFree( handle, pointer ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); MemoryAllocatorInstanceManager::SharedPoolAllocators[handle]->SharedDeallocate( pointer ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock()
#define pSharedDelete( handle, pointer ) MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.lock(); MemoryAllocatorInstanceManager::PoolAllocators[handle]->SharedDeallocate( pointer ); MemoryAllocatorInstanceManager::SharedPoolAllocatorsLock.unlock()
#else
#define InitializePoolAllocator( blockSize, blockCount, alignment ) INVALID_POOL_ALLOCATOR_HANDLE
#define ShutdownPoolAllocator( handle )

#define pMalloc( handle, count ) malloc( count )
#define pNew( handle, type, ... ) new type( __VA_ARGS__ )
#define pFree( handle, pointer ) free( pointer )
#define pDelete( handle, pointer ) delete pointer

#define InitializeSharedPoolAllocator( blockSize, blockCount, alignment ) INVALID_POOL_ALLOCATOR_HANDLE
#define ShutdownSharedPoolAllocator( handle )

#define pSharedMalloc( handle, count ) malloc( count )
#define pSharedNew( handle, type, ... ) new type( __VA_ARGS__ )
#define pSharedFree( handle, pointer ) free( pointer )
#define pSharedDelete( handle, pointer ) delete pointer
#endif