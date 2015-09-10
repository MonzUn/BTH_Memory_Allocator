#pragma once
#include <cstdlib>
#include <cassert>
#include <cstring>
#include "AllocatorUtility.h"

#define FRAME_ALLOCATOR_DEBUG 1
#define BUFFER_SIZE_BYTES_MEGA 16
#define BUFFER_SIZE_BYTES BUFFER_SIZE_BYTES_MEGA * 1024ULL * 1024ULL

class FrameAllocator
{
public:
	void Initialize()
	{
		m_Memory = static_cast<Byte*>( malloc( BUFFER_SIZE_BYTES ) );
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
		assert( m_Walker + sizeof( T ) < m_Memory + BUFFER_SIZE_BYTES );
#endif
		memcpy( m_Walker, &count, sizeof( size_t ) );

		Byte* returnPos = m_Walker + sizeof( size_t );

		size_t size = count * sizeof( T ) + sizeof( size_t );
		size += ( size & 0xF ) ? 16 - ( size & 0xF ) : 0; // 16-byte alignment
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
	Byte* m_Memory = nullptr;
	Byte* m_Walker = nullptr;
};