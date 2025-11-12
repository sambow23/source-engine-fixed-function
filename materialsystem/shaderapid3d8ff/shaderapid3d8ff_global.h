//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8 Fixed Function Shader API - Global definitions
//
//===========================================================================//

#ifndef SHADERAPID3D8FF_GLOBAL_H
#define SHADERAPID3D8FF_GLOBAL_H

#ifdef _WIN32
#pragma once
#endif

// Don't include togl/rendermechanism.h - we don't use OpenGL abstraction
// #include "togl/rendermechanism.h"

// Forward declarations
class IShaderUtil;
class IShaderShadow;
class IShaderAPI;
class IHardwareConfigInternal;
class IMaterialSystemHardwareConfig;

// Global interfaces
extern IShaderUtil* g_pShaderUtil;
extern IShaderShadow* g_pShaderShadow;
extern IHardwareConfigInternal* g_pHWConfig;
extern IMaterialSystemHardwareConfig* g_pMaterialSystemHardwareConfig;

//-----------------------------------------------------------------------------
// The main hardware config interface
//-----------------------------------------------------------------------------
inline IMaterialSystemHardwareConfig* HardwareConfig()
{	
	return g_pMaterialSystemHardwareConfig;
}

// D3D8 Fixed Function specific limits
#define D3D8FF_MAX_TEXTURE_STAGES 8
#define D3D8FF_MAX_LIGHTS 8
#define D3D8FF_MAX_VERTEX_STREAMS 8

// Feature flags for D3D8FF
#define D3D8FF_FEATURE_MULTITEXTURE		0x01
#define D3D8FF_FEATURE_ENVMAPPING		0x02
#define D3D8FF_FEATURE_LIGHTING			0x04
#define D3D8FF_FEATURE_FOG				0x08

#endif // SHADERAPID3D8FF_GLOBAL_H

