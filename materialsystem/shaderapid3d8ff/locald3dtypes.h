//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Local D3D types for D3D8FF
//
//===========================================================================//

#ifndef LOCALD3DTYPES_D3D8FF_H
#define LOCALD3DTYPES_D3D8FF_H

#ifdef _WIN32
#pragma once
#endif

// Use D3D9 headers but with fixed function subset
// Don't include d3dx9 to avoid COM interface issues
#if defined( _WIN32 ) && !defined( _X360 )
	#include <windows.h>
	#include "../../dx9sdk/include/d3d9.h"
#else
	#include <d3d9.h>
#endif

// Redefine some types for clarity
typedef D3DFORMAT D3D8FF_FORMAT;
typedef D3DPRIMITIVETYPE D3D8FF_PRIMITIVETYPE;
typedef D3DPOOL D3D8FF_POOL;
typedef D3DRESOURCETYPE D3D8FF_RESOURCETYPE;

#endif // LOCALD3DTYPES_D3D8FF_H

