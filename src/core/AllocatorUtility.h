#pragma once

// Nicer name when dealing with char as byte
typedef char Byte;
static_assert( sizeof( Byte ) == 1, "Sizeof Byte must be 1" );

#define KIBI ( 1024 )
#define MEBI ( 1024 * 1024 )