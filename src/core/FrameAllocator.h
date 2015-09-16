#pragma once
#include <cstdlib>
#include <cassert>
#include <cstring>
#include "AllocatorUtility.h"

#define FRAME_ALLOCATOR_DEBUG 1

class FrameAllocator
{
public:
	void Initialize( size_t memoryByteSize = 16 * MEBI, size_t alignment = 16 )
	{
		assert( alignment != 0 );
		assert( ( alignment & ( ~alignment + 1 ) ) == alignment );	// The alignment must be a power of 2

		assert( memoryByteSize >= alignment );						// The memory size must be at least one alignment big
		assert( memoryByteSize % alignment == 0 );					// The memory size must be a multiple of the alignment

		m_MemoryByteSize	= memoryByteSize;
		m_Alignment			= alignment;

		m_Memory = static_cast<Byte*>( malloc( memoryByteSize ) );
		m_Walker = m_Memory;
	}

	void Shutdown()
	{
		free( m_Memory );
	}

	void Reset()
	{
		m_Walker = m_Memory;
	}

	template<typename T>
	T* Allocate( size_t count = 1ULL)
	{
#if FRAME_ALLOCATOR_DEBUG == 1
		// Ensure that we don't run out of memory
		assert( m_Walker + sizeof( T ) < m_Memory + m_MemoryByteSize );
#endif
		memcpy( m_Walker, &count, sizeof( size_t ) );

		Byte* returnPos = m_Walker + sizeof( size_t );

		size_t size = count * sizeof( T ) + sizeof( size_t );
		size += ( size & m_Alignment ) ? m_Alignment - ( size & m_Alignment ) : 0; // Align the memory
		m_Walker += size;

		return reinterpret_cast<T*>( returnPos );
	}

	template<typename T>
	T* Create( size_t count )
	{
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
	Byte*	m_Memory = nullptr;
	Byte*	m_Walker = nullptr;

	size_t	m_MemoryByteSize;
	size_t	m_Alignment;
};