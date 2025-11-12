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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Singleton instances
//-----------------------------------------------------------------------------
IShaderUtil *g_pShaderUtil = NULL;
IShaderShadow *g_pShaderShadow = NULL;
IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig = NULL;
// Note: g_pShaderAPI is set in CShaderAPID3D8FF
// IHardwareConfigInternal *g_pHWConfig is set in device creation

//-----------------------------------------------------------------------------
// Main shader device manager instantiation
//-----------------------------------------------------------------------------
static CShaderDeviceMgrD3D8FF s_ShaderDeviceMgrD3D8FF;

//-----------------------------------------------------------------------------
// Exported interface
//-----------------------------------------------------------------------------
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderDeviceMgrD3D8FF, IShaderDeviceMgr,
	SHADER_DEVICE_MGR_INTERFACE_VERSION, s_ShaderDeviceMgrD3D8FF )

//-----------------------------------------------------------------------------
// Factory to return from SetMode
//-----------------------------------------------------------------------------
void* CShaderDeviceMgrBase::ShaderInterfaceFactory( const char *pInterfaceName, int *pReturnCode )
{
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_OK;
	}

	// Hardware config interface
	if ( !V_strcmp( pInterfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) )
	{
		return static_cast<IHardwareConfigInternal*>( g_pHardwareConfig );
	}

	// Shader API interface
	if ( !V_strcmp( pInterfaceName, SHADERAPI_INTERFACE_VERSION ) )
	{
		return static_cast<IShaderAPI*>( g_pShaderAPI );
	}

	// Shader device interface
	if ( !V_strcmp( pInterfaceName, SHADER_DEVICE_INTERFACE_VERSION ) )
	{
		return static_cast<IShaderDevice*>( g_pShaderDeviceD3D8FF );
	}

	// Shader shadow interface (to be implemented)
	if ( !V_strcmp( pInterfaceName, SHADERSHADOW_INTERFACE_VERSION ) )
	{
		return static_cast<IShaderShadow*>( g_pShaderShadow );
	}

	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// DLL entry point
//-----------------------------------------------------------------------------
#ifdef _WIN32
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	return TRUE;
}
#endif

