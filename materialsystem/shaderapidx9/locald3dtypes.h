//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef LOCALD3DTYPES_H
#define LOCALD3DTYPES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#if defined( DX10 ) && !defined( DX_TO_GL_ABSTRACTION )

#include <d3d10.h>
#include <d3dx10.h>

struct IDirect3D10BaseTexture
{
	ID3D10Resource			 *m_pBaseTexture;
	ID3D10ShaderResourceView *m_pSRView;
	ID3D10RenderTargetView	 *m_pRTView;
};

class CDx10Types
{
public:
	typedef struct IDirect3D10BaseTexture	IDirect3DTexture;
	// FIXME: What is this called now ? 
	// typedef ID3D10TextureCube			IDirect3DCubeTexture;
	typedef ID3D10Texture3D					IDirect3DVolumeTexture;
	typedef ID3D10Device					IDirect3DDevice;
	typedef D3D10_VIEWPORT					D3DVIEWPORT;
	typedef ID3D10Buffer					IDirect3DIndexBuffer;
	typedef ID3D10Buffer					IDirect3DVertexBuffer;
	typedef ID3D10VertexShader				IDirect3DVertexShader;
	typedef ID3D10PixelShader				IDirect3DPixelShader;
	typedef ID3D10ShaderResourceView		IDirect3DSurface;
	typedef ID3DX10Font						ID3DXFont;
	typedef ID3D10Query						ID3DQuery;

	typedef ID3D10Device					*LPDIRECT3DDEVICE;
	typedef ID3D10Buffer					*LPDIRECT3DINDEXBUFFER;
	typedef ID3D10Buffer					*LPDIRECT3DVERTEXBUFFER;
};

#endif // defined( DX10 ) && !defined( DX_TO_GL_ABSTRACTION )


#if !defined( _X360 ) && !defined( DX_TO_GL_ABSTRACTION )
#ifdef _DEBUG
#define D3D_DEBUG_INFO 1
#endif
#endif

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

typedef struct _D3DLIGHT9				D3DLIGHT9;
typedef struct _D3DADAPTER_IDENTIFIER9	D3DADAPTER_IDENTIFIER9;
typedef struct _D3DCAPS9				D3DCAPS9;
typedef struct _D3DVIEWPORT9			D3DVIEWPORT9;
typedef struct _D3DMATERIAL9			D3DMATERIAL9;
typedef IDirect3DTexture9				IDirect3DTexture;
typedef IDirect3DBaseTexture9			IDirect3DBaseTexture;
typedef IDirect3DCubeTexture9			IDirect3DCubeTexture;
typedef IDirect3DVolumeTexture9 		IDirect3DVolumeTexture;
typedef IDirect3DDevice9				IDirect3DDevice;
typedef D3DMATERIAL9					D3DMATERIAL;
typedef D3DLIGHT9						D3DLIGHT;
typedef IDirect3DSurface9				IDirect3DSurface;
typedef D3DCAPS9						D3DCAPS;
typedef IDirect3DIndexBuffer9			IDirect3DIndexBuffer;
typedef IDirect3DVertexBuffer9			IDirect3DVertexBuffer;
typedef IDirect3DPixelShader9			IDirect3DPixelShader;
typedef IDirect3DDevice					*LPDIRECT3DDEVICE;
typedef IDirect3DIndexBuffer			*LPDIRECT3DINDEXBUFFER;
typedef IDirect3DVertexBuffer			*LPDIRECT3DVERTEXBUFFER;

class CDx9Types
{
public:
	typedef IDirect3DTexture9				IDirect3DTexture;
	typedef IDirect3DBaseTexture9			IDirect3DBaseTexture;
	typedef IDirect3DCubeTexture9			IDirect3DCubeTexture;
	typedef IDirect3DVolumeTexture9 		IDirect3DVolumeTexture;
	typedef IDirect3DDevice9				IDirect3DDevice;
	typedef D3DMATERIAL9					D3DMATERIAL;
	typedef D3DLIGHT9						D3DLIGHT;
	typedef IDirect3DSurface9				IDirect3DSurface;
	typedef D3DCAPS9						D3DCAPS;
	typedef IDirect3DIndexBuffer9			IDirect3DIndexBuffer;
	typedef IDirect3DVertexBuffer9			IDirect3DVertexBuffer;
	typedef IDirect3DPixelShader9			IDirect3DPixelShader;
	typedef IDirect3DDevice					*LPDIRECT3DDEVICE;
	typedef IDirect3DIndexBuffer			*LPDIRECT3DINDEXBUFFER;
	typedef IDirect3DVertexBuffer			*LPDIRECT3DVERTEXBUFFER;
};

typedef void *HardwareShader_t;

//-----------------------------------------------------------------------------
// The vertex and pixel shader type
//-----------------------------------------------------------------------------
typedef intp VertexShader_t;
typedef intp PixelShader_t;

//-----------------------------------------------------------------------------
// Bitpattern for an invalid shader
//-----------------------------------------------------------------------------
#define INVALID_SHADER	(-1) // ( 0xFFFFFFFF )
#define INVALID_HARDWARE_SHADER ( NULL )

#define D3DSAMP_NOTSUPPORTED					D3DSAMP_FORCE_DWORD
#define D3DRS_NOTSUPPORTED						D3DRS_FORCE_DWORD

// Include appropriate headers based on backend
#if defined( USE_DXVK_NATIVE )
	// DXVK-native: use real DirectX headers from DXVK
	// Define NOMINMAX FIRST, before ANY includes
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
	
	// Note: DXVK_TYPES_DEFINED is set globally in wscript to prevent
	// platform.h/basetypes.h from defining Windows types that DXVK provides
	
	// ============================================================================
	// STEP 1: Include C/C++ standard library FIRST
	// ============================================================================
	#include <cstddef>
	#include <cstdint>
	
	// Include ONLY C headers to ensure functions are in global namespace
	// (Do NOT mix with C++ <cstring> which is more strict about namespaces)
	#include <string.h>   // strncpy, memcpy, etc.
	#include <ctype.h>    // isspace, etc.
	#include <stdlib.h>   // standard library
	#include <math.h>     // math functions
	
	// ============================================================================
	// STEP 2: Include DXVK's Windows and DirectX headers
	// Let DXVK define all the standard Windows types
	// ============================================================================
	#include <windows.h>  // DXVK's minimal Windows headers (GUID, RECT, etc.)
	#include <d3d9types.h>  // D3DFORMAT and other D3D9 types
	#include <d3d9.h>     // DirectX 9 API from DXVK
	
	// ============================================================================
	// STEP 3: Add missing types and Windows API functions that DXVK doesn't provide
	// ============================================================================
	
	// VD3DHWND - Valve's DirectX HWND type (same as HWND)
	typedef HWND VD3DHWND;
	
	// Note: MEMORYSTATUS is defined by DXVK's windows_base.h
	// DXVK's version is minimal (only dwLength and dwTotalPhys)
	// Full Windows version has 8 fields, but we'll use DXVK's minimal version
	
	// Windows API stubs for functions Source Engine uses but DXVK doesn't provide
	#ifndef GetClientRect
	inline BOOL GetClientRect(HWND hWnd, LPRECT lpRect) {
		// On Linux, this should never be called - SDL handles window management
		// But we need the symbol for compilation
		if (lpRect) {
			lpRect->left = 0;
			lpRect->top = 0;
			lpRect->right = 640;
			lpRect->bottom = 480;
		}
		return TRUE;
	}
	#endif
	
	// Note: ClientToScreen, IsIconic, GlobalMemoryStatus are implemented in winutils.cpp
	
	// ============================================================================
	// STEP 4: Minimal D3DX Math Compatibility Layer
	// DXVK doesn't provide D3DX (the helper library), only D3D9 API.
	// Define only the types and functions Source Engine actually uses.
	// ============================================================================
	
	// Forward declaration
	struct _D3DXMATRIX;
	typedef struct _D3DXMATRIX D3DXMATRIX;
	D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2);
	
	// D3DXVECTOR2 - 2D vector
	typedef struct _D3DXVECTOR2 {
		FLOAT x, y;
		_D3DXVECTOR2() {}
		_D3DXVECTOR2(FLOAT fx, FLOAT fy) : x(fx), y(fy) {}
		
		// Array access operator
		FLOAT& operator[](UINT i) { return (&x)[i]; }
		CONST FLOAT& operator[](UINT i) const { return (&x)[i]; }
		
		// Cast operators
		operator FLOAT*() { return &x; }
		operator CONST FLOAT*() const { return &x; }
	} D3DXVECTOR2;
	
	// D3DXVECTOR3 - 3D vector compatible with D3DVECTOR
	typedef struct _D3DXVECTOR3 : public D3DVECTOR {
		_D3DXVECTOR3() {}
		_D3DXVECTOR3(FLOAT fx, FLOAT fy, FLOAT fz) { x = fx; y = fy; z = fz; }
		
		// Array access operator (critical for Source Engine)
		FLOAT& operator[](UINT i) { return (&x)[i]; }
		CONST FLOAT& operator[](UINT i) const { return (&x)[i]; }
		
		// Cast operators
		operator FLOAT*() { return &x; }
		operator CONST FLOAT*() const { return &x; }
	} D3DXVECTOR3;
	
	// D3DXVECTOR4 - 4D vector
	typedef struct _D3DXVECTOR4 {
		FLOAT x, y, z, w;
		_D3DXVECTOR4() {}
		_D3DXVECTOR4(FLOAT fx, FLOAT fy, FLOAT fz, FLOAT fw) : x(fx), y(fy), z(fz), w(fw) {}
		
		// Array access operator
		FLOAT& operator[](UINT i) { return (&x)[i]; }
		CONST FLOAT& operator[](UINT i) const { return (&x)[i]; }
		
		// Cast operators
		operator FLOAT*() { return &x; }
		operator CONST FLOAT*() const { return &x; }
	} D3DXVECTOR4;
	
	// D3DXPLANE - plane equation
	typedef struct _D3DXPLANE {
		FLOAT a, b, c, d;
		_D3DXPLANE() {}
		_D3DXPLANE(FLOAT fa, FLOAT fb, FLOAT fc, FLOAT fd) : a(fa), b(fb), c(fc), d(fd) {}
		
		// Array access operator
		FLOAT& operator[](UINT i) { return (&a)[i]; }
		CONST FLOAT& operator[](UINT i) const { return (&a)[i]; }
		
		// Cast operators
		operator FLOAT*() { return &a; }
		operator CONST FLOAT*() const { return &a; }
		
		// Comparison operators
		bool operator==(CONST _D3DXPLANE& p) const { return a == p.a && b == p.b && c == p.c && d == p.d; }
		bool operator!=(CONST _D3DXPLANE& p) const { return a != p.a || b != p.b || c != p.c || d != p.d; }
	} D3DXPLANE;
	
	// D3DXMATRIX - 4x4 matrix compatible with D3DMATRIX
	struct _D3DXMATRIX : public D3DMATRIX {
		_D3DXMATRIX() {}
		_D3DXMATRIX(CONST FLOAT* pf) { memcpy(&_11, pf, sizeof(D3DMATRIX)); }
		_D3DXMATRIX(CONST D3DMATRIX& mat) { memcpy(this, &mat, sizeof(D3DMATRIX)); }
		
		// Access operators - parentheses for 2D access
		FLOAT& operator()(UINT row, UINT col) { return m[row][col]; }
		CONST FLOAT& operator()(UINT row, UINT col) const { return m[row][col]; }
		
		// Bracket operator for flat indexing (treat matrix as 16-element array)
		// This is needed for code like mat[3], mat[7], etc.
		FLOAT& operator[](UINT index) {
			return ((FLOAT*)&_11)[index];
		}
		CONST FLOAT& operator[](UINT index) const {
			return ((CONST FLOAT*)&_11)[index];
		}
		
		// Cast operators
		operator FLOAT*() { return (FLOAT*)&_11; }
		operator CONST FLOAT*() const { return (CONST FLOAT*)&_11; }
		
		// Comparison operators
		bool operator==(CONST _D3DXMATRIX& mat) const { return memcmp(this, &mat, sizeof(D3DMATRIX)) == 0; }
		bool operator!=(CONST _D3DXMATRIX& mat) const { return memcmp(this, &mat, sizeof(D3DMATRIX)) != 0; }
		
		// Math operators (friend functions for multiplication)
		friend _D3DXMATRIX operator*(CONST _D3DXMATRIX& m1, CONST _D3DXMATRIX& m2) {
			_D3DXMATRIX out;
			D3DXMatrixMultiply(&out, &m1, &m2);
			return out;
		}
	};
	
	// ID3DXMatrixStack interface (minimal stub for Source Engine compatibility)
	struct ID3DXMatrixStack {
		virtual HRESULT WINAPI QueryInterface(const IID&, void**) = 0;
		virtual ULONG WINAPI AddRef() = 0;
		virtual ULONG WINAPI Release() = 0;
		virtual HRESULT WINAPI Pop() = 0;
		virtual HRESULT WINAPI Push() = 0;
		virtual HRESULT WINAPI LoadIdentity() = 0;
		virtual HRESULT WINAPI LoadMatrix(CONST D3DMATRIX*) = 0;
		virtual HRESULT WINAPI MultMatrix(CONST D3DMATRIX*) = 0;
		virtual HRESULT WINAPI MultMatrixLocal(CONST D3DMATRIX*) = 0;
		virtual HRESULT WINAPI RotateAxis(CONST D3DXVECTOR3*, FLOAT) = 0;
		virtual HRESULT WINAPI RotateAxisLocal(CONST D3DXVECTOR3*, FLOAT) = 0;
		virtual HRESULT WINAPI RotateYawPitchRoll(FLOAT, FLOAT, FLOAT) = 0;
		virtual HRESULT WINAPI RotateYawPitchRollLocal(FLOAT, FLOAT, FLOAT) = 0;
		virtual HRESULT WINAPI Scale(FLOAT, FLOAT, FLOAT) = 0;
		virtual HRESULT WINAPI ScaleLocal(FLOAT, FLOAT, FLOAT) = 0;
		virtual HRESULT WINAPI Translate(FLOAT, FLOAT, FLOAT) = 0;
		virtual HRESULT WINAPI TranslateLocal(FLOAT, FLOAT, FLOAT) = 0;
		virtual D3DXMATRIX* WINAPI GetTop() = 0;
	};
	
	// D3DX Matrix Functions - declarations only (D3DXMatrixMultiply declared earlier)
	D3DXMATRIX* WINAPI D3DXMatrixIdentity(D3DXMATRIX *pOut);
	D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM);
	D3DXMATRIX* WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM);
	D3DXMATRIX* WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z);
	D3DXMATRIX* WINAPI D3DXMatrixPerspectiveRH(D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf);
	D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterRH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf);
	D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf);
	HRESULT WINAPI D3DXCreateMatrixStack(DWORD flags, ID3DXMatrixStack **ppStack);
	
	// D3DX Plane Functions
	D3DXPLANE* WINAPI D3DXPlaneTransform(D3DXPLANE *pOut, CONST D3DXPLANE *pP, CONST D3DXMATRIX *pM);
	D3DXPLANE* WINAPI D3DXPlaneNormalize(D3DXPLANE *pOut, CONST D3DXPLANE *pP);
	
	// D3DX Vector3 Functions (critical for Source Engine)
	D3DXVECTOR3* WINAPI D3DXVec3Normalize(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV);
	D3DXVECTOR3* WINAPI D3DXVec3Subtract(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2);
	D3DXVECTOR3* WINAPI D3DXVec3Cross(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2);
	FLOAT WINAPI D3DXVec3Dot(CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2);
	FLOAT WINAPI D3DXVec3Length(CONST D3DXVECTOR3 *pV);
	D3DXVECTOR3* WINAPI D3DXVec3Transform(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM);
	D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM);
	D3DXVECTOR3* WINAPI D3DXVec3TransformNormal(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM);
	
	// D3DX Vector4 Functions
	D3DXVECTOR4* WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM);
	D3DXVECTOR4* WINAPI D3DXVec4Normalize(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV);
	
	// D3DX Shader Compilation (needed for vertex/pixel shader compilation)
	struct ID3DXBuffer : public IUnknown {
		virtual LPVOID WINAPI GetBufferPointer() = 0;
		virtual DWORD WINAPI GetBufferSize() = 0;
	};
	typedef ID3DXBuffer* LPD3DXBUFFER;
	
	// D3DXMACRO structure for shader macros
	typedef struct _D3DXMACRO {
		LPCSTR Name;
		LPCSTR Definition;
	} D3DXMACRO;
	
	// ID3DXInclude interface for shader includes
	struct ID3DXInclude {
		virtual HRESULT WINAPI Open(UINT, LPCSTR, LPCVOID, LPCVOID*, UINT*) = 0;
		virtual HRESULT WINAPI Close(LPCVOID) = 0;
	};
	typedef ID3DXInclude* LPD3DXINCLUDE;
	
	// ID3DXConstantTable interface (minimal - Source Engine might not use this directly)
	struct ID3DXConstantTable : public IUnknown {
		// Minimal interface - add methods as needed
	};
	typedef ID3DXConstantTable* LPD3DXCONSTANTTABLE;
	
	// Shader compilation flags
	#define D3DXSHADER_AVOID_FLOW_CONTROL (1 << 9)
	
	// Shader compilation functions
	HRESULT WINAPI D3DXCompileShader(
		LPCSTR pSrcData,
		UINT SrcDataLen,
		CONST D3DXMACRO* pDefines,
		LPD3DXINCLUDE pInclude,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		DWORD Flags,
		LPD3DXBUFFER* ppShader,
		LPD3DXBUFFER* ppErrorMsgs,
		LPD3DXCONSTANTTABLE* ppConstantTable);
	
	DWORD WINAPI D3DXGetShaderVersion(CONST DWORD* pFunction);
	
	// min() function - define inline to avoid conflicts
	static inline float min(float a, float b) { return (a < b) ? a : b; }
	static inline int min(int a, int b) { return (a < b) ? a : b; }
	static inline unsigned int min(unsigned int a, unsigned int b) { return (a < b) ? a : b; }
	
	// Note: Actual D3DX implementations will need to be provided at runtime
	// Either by linking against Wine's d3dx9 library, or by implementing them
	
	// Forcibly undef min/max macros just in case
	#ifdef min
	#undef min
	#endif
	#ifdef max
	#undef max
	#endif
	
	// DXVK provides native DirectX D3D9 implementation via Vulkan
	
#elif defined( DX_TO_GL_ABSTRACTION )
	// ToGL: use OpenGL wrapper
	#include "togl/rendermechanism.h"
#else
	// Windows: use real DirectX from local SDK
	#if defined( _WIN32 ) && !defined( _X360 )
		#include "../../dx9sdk/include/d3d9.h"
		#include "../../dx9sdk/include/d3dx9.h"
	#else
		#include <d3d9.h>
		#include <d3dx9.h>
	#endif
#endif

#if defined( _X360 )

// not supported, keeping for port ease
#define D3DSAMP_SRGBTEXTURE						D3DSAMP_NOTSUPPORTED
#define D3DRS_LIGHTING							D3DRS_NOTSUPPORTED
#define D3DRS_DIFFUSEMATERIALSOURCE				D3DRS_NOTSUPPORTED
#define D3DRS_SPECULARENABLE					D3DRS_NOTSUPPORTED
#define D3DRS_SHADEMODE							D3DRS_NOTSUPPORTED
#define D3DRS_LASTPIXEL							D3DRS_NOTSUPPORTED
#define D3DRS_DITHERENABLE						D3DRS_NOTSUPPORTED
#define D3DRS_FOGENABLE							D3DRS_NOTSUPPORTED
#define D3DRS_FOGCOLOR							D3DRS_NOTSUPPORTED
#define D3DRS_FOGTABLEMODE						D3DRS_NOTSUPPORTED
#define D3DRS_FOGSTART							D3DRS_NOTSUPPORTED
#define D3DRS_FOGEND							D3DRS_NOTSUPPORTED
#define D3DRS_FOGDENSITY						D3DRS_NOTSUPPORTED
#define D3DRS_RANGEFOGENABLE					D3DRS_NOTSUPPORTED
#define D3DRS_TEXTUREFACTOR						D3DRS_NOTSUPPORTED
#define D3DRS_CLIPPING							D3DRS_NOTSUPPORTED
#define D3DRS_AMBIENT							D3DRS_NOTSUPPORTED
#define D3DRS_FOGVERTEXMODE						D3DRS_NOTSUPPORTED
#define D3DRS_COLORVERTEX						D3DRS_NOTSUPPORTED
#define D3DRS_LOCALVIEWER						D3DRS_NOTSUPPORTED
#define D3DRS_NORMALIZENORMALS					D3DRS_NOTSUPPORTED
#define D3DRS_SPECULARMATERIALSOURCE			D3DRS_NOTSUPPORTED
#define D3DRS_AMBIENTMATERIALSOURCE				D3DRS_NOTSUPPORTED
#define D3DRS_EMISSIVEMATERIALSOURCE			D3DRS_NOTSUPPORTED	
#define D3DRS_VERTEXBLEND						D3DRS_NOTSUPPORTED
#define D3DRS_POINTSCALEENABLE					D3DRS_NOTSUPPORTED
#define D3DRS_POINTSCALE_A						D3DRS_NOTSUPPORTED
#define D3DRS_POINTSCALE_B						D3DRS_NOTSUPPORTED
#define D3DRS_POINTSCALE_C						D3DRS_NOTSUPPORTED
#define D3DRS_PATCHEDGESTYLE					D3DRS_NOTSUPPORTED
#define D3DRS_DEBUGMONITORTOKEN					D3DRS_NOTSUPPORTED
#define D3DRS_INDEXEDVERTEXBLENDENABLE			D3DRS_NOTSUPPORTED
#define D3DRS_TWEENFACTOR						D3DRS_NOTSUPPORTED
#define D3DRS_POSITIONDEGREE					D3DRS_NOTSUPPORTED
#define D3DRS_NORMALDEGREE						D3DRS_NOTSUPPORTED
#define D3DRS_ANTIALIASEDLINEENABLE				D3DRS_NOTSUPPORTED
#define D3DRS_ADAPTIVETESS_X					D3DRS_NOTSUPPORTED
#define D3DRS_ADAPTIVETESS_Y					D3DRS_NOTSUPPORTED
#define D3DRS_ADAPTIVETESS_Z					D3DRS_NOTSUPPORTED
#define D3DRS_ADAPTIVETESS_W					D3DRS_NOTSUPPORTED
#define D3DRS_ENABLEADAPTIVETESSELLATION		D3DRS_NOTSUPPORTED
#define D3DRS_SRGBWRITEENABLE					D3DRS_NOTSUPPORTED
#define D3DLOCK_DISCARD							0
#define D3DUSAGE_DYNAMIC						0
#define D3DUSAGE_AUTOGENMIPMAP					0
#define D3DDEVTYPE_REF							D3DDEVTYPE_HAL
#define D3DENUM_WHQL_LEVEL						0
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING		D3DCREATE_HARDWARE_VERTEXPROCESSING
#define D3DDMT_ENABLE							0

typedef enum D3DSHADEMODE
{
	D3DSHADE_FLAT = 0,
	D3DSHADE_GOURAUD = 0,
};

#endif // _X360

#endif // LOCALD3DTYPES_H
