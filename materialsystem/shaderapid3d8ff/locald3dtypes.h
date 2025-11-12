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

#include "tier0/platform.h"

// Use D3D9 headers but with fixed function subset
#if defined( _WIN32 ) && !defined( _X360 )
	#include <windows.h>
	#include "../../dx9sdk/include/d3d9.h"
	#include "../../dx9sdk/include/d3dx9.h"
#else
	#include <d3d9.h>
	#include <d3dx9.h>
#endif

// Forward declarations for D3D9 interfaces
struct IDirect3DTexture9;
struct IDirect3DBaseTexture9;
struct IDirect3DCubeTexture9;
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexBuffer9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DVolumeTexture9;
struct IDirect3DVertexDeclaration9;

// Typedefs for D3D8FF - use D3D9 types
typedef IDirect3DTexture9 IDirect3DTexture;
typedef IDirect3DBaseTexture9 IDirect3DBaseTexture;
typedef IDirect3DCubeTexture9 IDirect3DCubeTexture;
typedef IDirect3DVolumeTexture9 IDirect3DVolumeTexture;
typedef IDirect3DDevice9 IDirect3DDevice;
typedef IDirect3DSurface9 IDirect3DSurface;
typedef IDirect3DIndexBuffer9 IDirect3DIndexBuffer;
typedef IDirect3DVertexBuffer9 IDirect3DVertexBuffer;
typedef IDirect3DPixelShader9 IDirect3DPixelShader;
typedef IDirect3DVertexShader9 IDirect3DVertexShader;
typedef IDirect3DVertexDeclaration9 IDirect3DVertexDeclaration;

typedef IDirect3DDevice* LPDIRECT3DDEVICE;
typedef IDirect3DIndexBuffer* LPDIRECT3DINDEXBUFFER;
typedef IDirect3DVertexBuffer* LPDIRECT3DVERTEXBUFFER;

// Additional D3D9 types
typedef struct _D3DLIGHT9 D3DLIGHT9;
typedef struct _D3DLIGHT9 D3DLIGHT;
typedef struct _D3DADAPTER_IDENTIFIER9 D3DADAPTER_IDENTIFIER9;
typedef struct _D3DCAPS9 D3DCAPS9;
typedef struct _D3DCAPS9 D3DCAPS;
typedef struct _D3DVIEWPORT9 D3DVIEWPORT9;
typedef struct _D3DMATERIAL9 D3DMATERIAL9;
typedef struct _D3DMATERIAL9 D3DMATERIAL;

// Redefine some types for clarity
typedef D3DFORMAT D3D8FF_FORMAT;
typedef D3DPRIMITIVETYPE D3D8FF_PRIMITIVETYPE;
typedef D3DPOOL D3D8FF_POOL;
typedef D3DRESOURCETYPE D3D8FF_RESOURCETYPE;

// Hardware shader types
typedef void *HardwareShader_t;
typedef intp VertexShader_t;
typedef intp PixelShader_t;
#define INVALID_SHADER (-1)
#define INVALID_HARDWARE_SHADER (NULL)

#endif // LOCALD3DTYPES_D3D8FF_H

