//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Shader API implementation - Main entry point
//
//===========================================================================//

#include "shaderapid3d8ff.h"
#include "shaderdeviced3d8ff.h"
#include "hardwareconfig.h"
#include "tier0/dbg.h"
#include "tier1/interface.h"
#include "tier0/icommandline.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Singleton instances
//-----------------------------------------------------------------------------
// g_pShaderUtil is defined in shaderdevicebase.cpp
// g_pShaderShadow is defined in shaderdevicebase.cpp
// g_pMaterialSystemHardwareConfig is defined in tier2.cpp
IShaderAPI *g_pShaderAPI = NULL;
IHardwareConfigInternal *g_pHWConfig = NULL;
// g_pShaderDeviceD3D8FF is defined in shaderdeviced3d8ff.cpp

//-----------------------------------------------------------------------------
// ConVars required by hardwareconfig.cpp
//-----------------------------------------------------------------------------
ConVar mat_hdr_level( "mat_hdr_level", "0", FCVAR_ARCHIVE ); // D3D8FF doesn't support HDR
ConVar mat_fastclip( "mat_fastclip", "0" );

//-----------------------------------------------------------------------------
// Main shader device manager instantiation
//-----------------------------------------------------------------------------
static CShaderDeviceMgrD3D8FF s_ShaderDeviceMgrD3D8FF;

//-----------------------------------------------------------------------------
// Exported interface
//-----------------------------------------------------------------------------
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderDeviceMgrD3D8FF, IShaderDeviceMgr,
	SHADER_DEVICE_MGR_INTERFACE_VERSION, s_ShaderDeviceMgrD3D8FF )

// ShaderInterfaceFactory is implemented in shaderdevicebase.cpp

//-----------------------------------------------------------------------------
// DLL entry point
//-----------------------------------------------------------------------------
#ifdef _WIN32
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	return TRUE;
}
#endif

