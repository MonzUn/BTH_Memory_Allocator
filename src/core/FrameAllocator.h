#pragma once
#include <stdlib.h>
#include <cassert>

// Nicer name when dealing with char as byte
typedef char Byte;
static_assert( sizeof( Byte ) == 1, "Sizeof Byte must be 1" );

//#define DISABLE_FRAME_ALLOCATOR

#ifndef DISABLE_FRAME_ALLOCATOR
#define fMalloc( count ) FrameAllocator::Allocate<Byte>( count )
#define fNew( type, ... ) new( FrameAllocator::Allocate<type>( 1 ) ) type( __VA_ARGS__ )
#define fFree( pointer )
#define fDelete( pointer ) // TODODB: Maybe we can just call the destructor here to be nice? :3
#else
#define fMalloc( count ) malloc( count )
#define fNew( type, ... ) new type( __VA_ARGS__ )
#define fFree( pointer ) free( pointer )
#define fDelete( pointer ) delete( pointer )
#endif

#define FRAME_ALLOCATOR_DEBUG 1
#define BUFFER_SIZE_BYTES_MEGA 16
#define BUFFER_SIZE_BYTES BUFFER_SIZE_BYTES_MEGA * 1024ULL * 1024ULL

namespace FrameAllocator 
{
	static Byte* memory = nullptr;
	static Byte* walker = nullptr;

	void Initialize()
	{
		memory = static_cast<Byte*>( malloc( BUFFER_SIZE_BYTES ) );
		walker = memory;
	}

	void Shutdown()
	{
		free( memory );
	}

	void Reset()
	{
		walker = memory;
		// TODODB: Mayhaps call destructors of allocated objects here? (Should a frame allocator even do this? O.o)
	}

	template<typename T>
	T* Allocate( size_t count = 1ULL)
	{
#if FRAME_ALLOCATOR_DEBUG == 1
		// Ensure that we don't run out of memory
		assert( walker + sizeof( T ) < memory + BUFFER_SIZE_BYTES );
#endif
		Byte* returnPos = walker;

		size_t size = count * sizeof( T );
		size += ( size & 0xF ) ? 16 - ( size & 0xF ) : 0; // 16-byte alignment
		walker += size;

		return reinterpret_cast<T*>( returnPos );
	}
}