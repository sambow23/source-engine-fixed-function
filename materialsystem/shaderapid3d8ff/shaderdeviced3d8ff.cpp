//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Shader Device implementation
//
//===========================================================================//

#include "shaderdeviced3d8ff.h"
#include "hardwareconfig.h"
#include "shaderapid3d8ff.h"
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier1/strtools.h"
#include "tier1/utlbuffer.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
IDirect3D9 *g_pD3D = NULL;
IDirect3DDevice9 *g_pD3DDevice = NULL;
CShaderDeviceMgrD3D8FF *g_pShaderDeviceMgrD3D8FF = NULL;
CShaderDeviceD3D8FF *g_pShaderDeviceD3D8FF = NULL;

//-----------------------------------------------------------------------------
// Device Manager implementation
//-----------------------------------------------------------------------------
CShaderDeviceMgrD3D8FF::CShaderDeviceMgrD3D8FF()
{
	m_pD3D = NULL;
	m_bAdapterInfoInitialized = false;
}

CShaderDeviceMgrD3D8FF::~CShaderDeviceMgrD3D8FF()
{
}

//-----------------------------------------------------------------------------
// Connect/Disconnect
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrD3D8FF::Connect( CreateInterfaceFn factory )
{
	if ( !BaseClass::Connect( factory ) )
		return false;

	// Create D3D9 interface
	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !m_pD3D )
	{
		Warning( "Failed to create Direct3D9 interface for D3D8FF backend\n" );
		return false;
	}

	g_pD3D = m_pD3D;
	g_pShaderDeviceMgrD3D8FF = this;

	return true;
}

void CShaderDeviceMgrD3D8FF::Disconnect()
{
	if ( m_pD3D )
	{
		m_pD3D->Release();
		m_pD3D = NULL;
		g_pD3D = NULL;
	}

	g_pShaderDeviceMgrD3D8FF = NULL;

	BaseClass::Disconnect();
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
InitReturnVal_t CShaderDeviceMgrD3D8FF::Init()
{
	// Don't call BaseClass::Init() - IAppSystem::Init() is pure virtual
	// Our initialization is done in Connect() and InitAdapterInfo()
	
	if ( !DetermineHardwareCaps() )
		return INIT_FAILED;

	Msg( "D3D8FF: Initialized fixed function backend\n" );
	
	return INIT_OK;
}

void CShaderDeviceMgrD3D8FF::Shutdown()
{
	// Don't call BaseClass::Shutdown() - IAppSystem::Shutdown() is pure virtual
	// Cleanup is handled by our own ShutdownDevice() and destructor
}

//-----------------------------------------------------------------------------
// Adapter info
//-----------------------------------------------------------------------------
int CShaderDeviceMgrD3D8FF::GetAdapterCount() const
{
	return m_pD3D ? m_pD3D->GetAdapterCount() : 0;
}

void CShaderDeviceMgrD3D8FF::InitAdapterInfo()
{
	if ( m_bAdapterInfoInitialized )
		return;

	m_bAdapterInfoInitialized = true;

	int nCount = GetAdapterCount();
	m_Adapters.SetSize( nCount );

	for ( int i = 0; i < nCount; ++i )
	{
		D3DADAPTER_IDENTIFIER9 ident;
		m_pD3D->GetAdapterIdentifier( i, 0, &ident );
		
		// Compute hardware caps
		HardwareCaps_t caps;
		ComputeCapsFromD3D( &caps, i );
		
		// Force to DX8 level for fixed function
		caps.m_nDXSupportLevel = 80;
		caps.m_nMaxDXSupportLevel = 80;
		// Note: m_nMinDXSupportLevel doesn't exist in HardwareCaps_t
		caps.m_SupportsPixelShaders = false;
		caps.m_SupportsVertexShaders = false;
		
		m_Adapters[i].m_ActualCaps = caps;
	}
}

void CShaderDeviceMgrD3D8FF::GetAdapterInfo( int nAdapter, MaterialAdapterInfo_t& info ) const
{
	Assert( m_pD3D && ( nAdapter < GetAdapterCount() ) );

	D3DADAPTER_IDENTIFIER9 ident;
	HRESULT hr = m_pD3D->GetAdapterIdentifier( nAdapter, 0, &ident );
	
	if ( SUCCEEDED( hr ) )
	{
		Q_strncpy( info.m_pDriverName, ident.Description, MATERIAL_ADAPTER_NAME_LENGTH );
		info.m_VendorID = ident.VendorId;
		info.m_DeviceID = ident.DeviceId;
		info.m_SubSysID = ident.SubSysId;
		info.m_Revision = ident.Revision;
		info.m_nDXSupportLevel = 80; // Fixed function = DX8
		info.m_nMaxDXSupportLevel = 80;
		info.m_nDriverVersionHigh = ident.DriverVersion.HighPart;
		info.m_nDriverVersionLow = ident.DriverVersion.LowPart;
	}
}

//-----------------------------------------------------------------------------
// Determines hardware caps from D3D
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrD3D8FF::ComputeCapsFromD3D( HardwareCaps_t *pCaps, int nAdapter )
{
	Assert( m_pD3D );

	D3DCAPS9 caps;
	HRESULT hr = m_pD3D->GetDeviceCaps( nAdapter, D3D8FF_DEVTYPE, &caps );
	if ( FAILED( hr ) )
		return false;

	D3DADAPTER_IDENTIFIER9 ident;
	hr = m_pD3D->GetAdapterIdentifier( nAdapter, 0, &ident );
	if ( FAILED( hr ) )
		return false;

	// Fill in caps structure
	memset( pCaps, 0, sizeof( HardwareCaps_t ) );

	// Basic info
	pCaps->m_VendorID = ident.VendorId;
	pCaps->m_DeviceID = ident.DeviceId;
	pCaps->m_SubSysID = ident.SubSysId;
	pCaps->m_Revision = ident.Revision;
	Q_strncpy( pCaps->m_pDriverName, ident.Description, sizeof( pCaps->m_pDriverName ) );
	pCaps->m_nDriverVersionHigh = ident.DriverVersion.HighPart;
	pCaps->m_nDriverVersionLow = ident.DriverVersion.LowPart;

	// Fixed function capabilities
	pCaps->m_NumTextureStages = min( (int)caps.MaxTextureBlendStages, D3D8FF_MAX_TEXTURE_STAGES );
	pCaps->m_NumSamplers = min( (int)caps.MaxSimultaneousTextures, D3D8FF_MAX_TEXTURE_STAGES );
	pCaps->m_MaxTextureWidth = caps.MaxTextureWidth;
	pCaps->m_MaxTextureHeight = caps.MaxTextureHeight;
	pCaps->m_MaxTextureDepth = caps.MaxVolumeExtent;
	pCaps->m_MaxTextureAspectRatio = caps.MaxTextureAspectRatio;
	pCaps->m_MaxPrimitiveCount = caps.MaxPrimitiveCount;
	
	// Lighting
	pCaps->m_SupportsHardwareLighting = ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0;
	// Note: m_MaxActiveLights doesn't exist in HardwareCaps_t, lighting is handled separately
	
	// Anisotropic filtering
	// Note: m_SupportsAnisotropicFiltering doesn't exist in HardwareCaps_t
	pCaps->m_nMaxAnisotropy = caps.MaxAnisotropy;
	
	// Cube maps
	pCaps->m_SupportsCubeMaps = ( caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP ) != 0;
	pCaps->m_SupportsMipmappedCubemaps = ( caps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP ) != 0;
	
	// Non-power-of-2 textures
	pCaps->m_SupportsNonPow2Textures = ( caps.TextureCaps & D3DPTEXTURECAPS_POW2 ) == 0;
	
	// Vertex processing
	// Note: m_nMaxVertexShaderBlendMatrices and m_nMaxUserClipPlanes don't exist in HardwareCaps_t
	pCaps->m_bSoftwareVertexProcessing = ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0;
	
	// Gamma ramp
	pCaps->m_HasSetDeviceGammaRamp = ( caps.Caps2 & D3DCAPS2_CANCALIBRATEGAMMA ) != 0;
	
	// No shader support in fixed function
	pCaps->m_SupportsPixelShaders = false;
	pCaps->m_SupportsVertexShaders = false;
	// Note: m_NumPixelShaderRenderTargets and m_SupportsVertexTextures don't exist in HardwareCaps_t
	
	// DX level
	ComputeDXSupportLevel( *pCaps );
	
	return true;
}

//-----------------------------------------------------------------------------
// Compute DX support level
//-----------------------------------------------------------------------------
void CShaderDeviceMgrD3D8FF::ComputeDXSupportLevel( HardwareCaps_t &caps )
{
	// Fixed function backend reports DX8 level
	caps.m_nDXSupportLevel = 80;
	caps.m_nMaxDXSupportLevel = 80;
	// Note: m_nMinDXSupportLevel doesn't exist in HardwareCaps_t
	
	Msg( "D3D8FF: Reporting DX8.0 fixed function capabilities\n" );
}

//-----------------------------------------------------------------------------
// Mode info
//-----------------------------------------------------------------------------
int CShaderDeviceMgrD3D8FF::GetModeCount( int nAdapter ) const
{
	Assert( m_pD3D && ( nAdapter < GetAdapterCount() ) );
	
	// Get desktop format
	D3DDISPLAYMODE mode;
	m_pD3D->GetAdapterDisplayMode( nAdapter, &mode );
	
	return m_pD3D->GetAdapterModeCount( nAdapter, mode.Format );
}

void CShaderDeviceMgrD3D8FF::GetModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter, int nMode ) const
{
	Assert( m_pD3D && pInfo && ( nAdapter < GetAdapterCount() ) );
	
	// Get desktop format
	D3DDISPLAYMODE desktopMode;
	m_pD3D->GetAdapterDisplayMode( nAdapter, &desktopMode );
	
	D3DDISPLAYMODE mode;
	HRESULT hr = m_pD3D->EnumAdapterModes( nAdapter, desktopMode.Format, nMode, &mode );
	
	if ( SUCCEEDED( hr ) )
	{
		pInfo->m_nWidth = mode.Width;
		pInfo->m_nHeight = mode.Height;
		pInfo->m_Format = ImageLoader::D3DFormatToImageFormat( mode.Format );
		pInfo->m_nRefreshRateNumerator = mode.RefreshRate;
		pInfo->m_nRefreshRateDenominator = 1;
	}
}

void CShaderDeviceMgrD3D8FF::GetCurrentModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter ) const
{
	Assert( m_pD3D && pInfo && ( nAdapter < GetAdapterCount() ) );
	
	D3DDISPLAYMODE mode;
	HRESULT hr = m_pD3D->GetAdapterDisplayMode( nAdapter, &mode );
	
	if ( SUCCEEDED( hr ) )
	{
		pInfo->m_nWidth = mode.Width;
		pInfo->m_nHeight = mode.Height;
		pInfo->m_Format = ImageLoader::D3DFormatToImageFormat( mode.Format );
		pInfo->m_nRefreshRateNumerator = mode.RefreshRate;
		pInfo->m_nRefreshRateDenominator = 1;
	}
}

//-----------------------------------------------------------------------------
// Sets the adapter
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrD3D8FF::SetAdapter( int nAdapter, int nFlags )
{
	InitAdapterInfo();
	
	if ( nAdapter < 0 || nAdapter >= GetAdapterCount() )
		return false;
	
	// Read hardware caps from dxsupport.cfg
	const HardwareCaps_t &actualCaps = m_Adapters[nAdapter].m_ActualCaps;
	ReadHardwareCaps( const_cast<HardwareCaps_t&>( actualCaps ), actualCaps.m_nDXSupportLevel );
	
	// Set up hardware config (cast to CHardwareConfig to access internal methods)
	((CHardwareConfig*)g_pHWConfig)->SetupHardwareCaps( actualCaps.m_nDXSupportLevel, actualCaps );
	
	return true;
}

//-----------------------------------------------------------------------------
// Validates the mode
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrD3D8FF::ValidateMode( int nAdapter, const ShaderDeviceInfo_t &info ) const
{
	if ( nAdapter < 0 || nAdapter >= GetAdapterCount() )
		return false;
	
	// Check if format is supported
	D3DFORMAT d3dFormat = (D3DFORMAT)ImageLoader::ImageFormatToD3DFormat( info.m_DisplayMode.m_Format );
	HRESULT hr = m_pD3D->CheckDeviceType( nAdapter, D3D8FF_DEVTYPE, d3dFormat, d3dFormat, info.m_bWindowed );
	
	return SUCCEEDED( hr );
}

//-----------------------------------------------------------------------------
// Video memory
//-----------------------------------------------------------------------------
int CShaderDeviceMgrD3D8FF::GetVidMemBytes( int nAdapter ) const
{
	if ( !m_pD3D || nAdapter < 0 || nAdapter >= GetAdapterCount() )
		return 0;
	
	// D3D9 doesn't have GetAvailableTextureMem on the D3D object, only on the device
	// We'll return the texture memory size from our caps structure
	return m_Adapters[nAdapter].m_ActualCaps.m_TextureMemorySize;
}

//-----------------------------------------------------------------------------
// Sets the mode and creates the device
//-----------------------------------------------------------------------------
CreateInterfaceFn CShaderDeviceMgrD3D8FF::SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode )
{
	if ( !ValidateMode( nAdapter, mode ) )
	{
		Warning( "D3D8FF: Invalid display mode\n" );
		return NULL;
	}
	
	// Create the device
	if ( !g_pShaderDeviceD3D8FF )
	{
		g_pShaderDeviceD3D8FF = new CShaderDeviceD3D8FF;
	}
	
	if ( !g_pShaderDeviceD3D8FF->InitDevice( hWnd, nAdapter, mode ) )
	{
		Warning( "D3D8FF: Failed to initialize device\n" );
		delete g_pShaderDeviceD3D8FF;
		g_pShaderDeviceD3D8FF = NULL;
		return NULL;
	}
	
	return ShaderInterfaceFactory;
}

//-----------------------------------------------------------------------------
// Determine hardware caps
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrD3D8FF::DetermineHardwareCaps()
{
	if ( !m_pD3D )
		return false;
	
	InitAdapterInfo();
	return true;
}

//-----------------------------------------------------------------------------
// Device implementation
//-----------------------------------------------------------------------------
CShaderDeviceD3D8FF::CShaderDeviceD3D8FF()
{
	m_DisplayAdapter = 0;
	m_DeviceType = D3D8FF_DEVTYPE;
	m_DeviceState = DEVICE_STATE_OK;
	m_bOtherAppInitializing = false;
	m_bQueuedDeviceLost = false;
	m_IsResizing = false;
	m_bPendingVideoModeChange = false;
	m_bUsingStencil = false;
	m_bResourcesReleased = false;
	m_iStencilBufferBits = 0;
	m_numReleaseResourcesRefCount = 0;
}

CShaderDeviceD3D8FF::~CShaderDeviceD3D8FF()
{
}

//-----------------------------------------------------------------------------
// Device queries
//-----------------------------------------------------------------------------
bool CShaderDeviceD3D8FF::IsUsingGraphics() const
{
	return g_pD3DDevice != NULL;
}

ImageFormat CShaderDeviceD3D8FF::GetBackBufferFormat() const
{
	return m_AdapterFormat;
}

void CShaderDeviceD3D8FF::GetBackBufferDimensions( int& width, int& height ) const
{
	width = m_PresentParameters.BackBufferWidth;
	height = m_PresentParameters.BackBufferHeight;
}

int CShaderDeviceD3D8FF::GetCurrentAdapter() const
{
	return m_DisplayAdapter;
}

char *CShaderDeviceD3D8FF::GetDisplayDeviceName()
{
	return (char*)m_sDisplayDeviceName.String();
}

//-----------------------------------------------------------------------------
// Present the back buffer
//-----------------------------------------------------------------------------
void CShaderDeviceD3D8FF::Present()
{
	if ( !g_pD3DDevice )
		return;
	
	HRESULT hr = g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
	
	if ( hr == D3DERR_DEVICELOST )
	{
		MarkDeviceLost();
	}
}

// Stub implementations for shader creation (not supported in fixed function)
IShaderBuffer* CShaderDeviceD3D8FF::CompileShader( const char *pProgram, size_t nBufLen, const char *pShaderVersion )
{
	// Fixed function doesn't use compiled shaders
	return NULL;
}

VertexShaderHandle_t CShaderDeviceD3D8FF::CreateVertexShader( IShaderBuffer *pBuffer )
{
	return VERTEX_SHADER_HANDLE_INVALID;
}

void CShaderDeviceD3D8FF::DestroyVertexShader( VertexShaderHandle_t hShader )
{
}

GeometryShaderHandle_t CShaderDeviceD3D8FF::CreateGeometryShader( IShaderBuffer* pShaderBuffer )
{
	return GEOMETRY_SHADER_HANDLE_INVALID;
}

void CShaderDeviceD3D8FF::DestroyGeometryShader( GeometryShaderHandle_t hShader )
{
}

PixelShaderHandle_t CShaderDeviceD3D8FF::CreatePixelShader( IShaderBuffer* pShaderBuffer )
{
	return PIXEL_SHADER_HANDLE_INVALID;
}

void CShaderDeviceD3D8FF::DestroyPixelShader( PixelShaderHandle_t hShader )
{
}

// To be implemented in subsequent files...
void CShaderDeviceD3D8FF::ReleaseResources() {}
void CShaderDeviceD3D8FF::ReacquireResources() {}
IMesh* CShaderDeviceD3D8FF::CreateStaticMesh( VertexFormat_t format, const char *pBudgetGroup, IMaterial * pMaterial ) { return NULL; }
void CShaderDeviceD3D8FF::DestroyStaticMesh( IMesh* mesh ) {}
IVertexBuffer *CShaderDeviceD3D8FF::CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup ) { return NULL; }
void CShaderDeviceD3D8FF::DestroyVertexBuffer( IVertexBuffer *pVertexBuffer ) {}
IIndexBuffer *CShaderDeviceD3D8FF::CreateIndexBuffer( ShaderBufferType_t bufferType, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup ) { return NULL; }
void CShaderDeviceD3D8FF::DestroyIndexBuffer( IIndexBuffer *pIndexBuffer ) {}
IVertexBuffer *CShaderDeviceD3D8FF::GetDynamicVertexBuffer( int nStreamID, VertexFormat_t vertexFormat, bool bBuffered ) { return NULL; }
IIndexBuffer *CShaderDeviceD3D8FF::GetDynamicIndexBuffer( MaterialIndexFormat_t fmt, bool bBuffered ) { return NULL; }
void CShaderDeviceD3D8FF::SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax, float fGammaTVExponent, bool bTVEnabled ) {}
void CShaderDeviceD3D8FF::SpewDriverInfo() const {}
void CShaderDeviceD3D8FF::EnableNonInteractiveMode( MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo ) {}
void CShaderDeviceD3D8FF::RefreshFrontBufferNonInteractive() {}
bool CShaderDeviceD3D8FF::InitDevice( void* hWnd, int nAdapter, const ShaderDeviceInfo_t &info ) { return false; }
void CShaderDeviceD3D8FF::ShutdownDevice() {}
void CShaderDeviceD3D8FF::OtherAppInitializing( bool initializing ) {}
void CShaderDeviceD3D8FF::HandleThreadEvent( uint32 threadEvent ) {}
bool CShaderDeviceD3D8FF::CreateD3DDevice( void* pHWnd, int nAdapter, const ShaderDeviceInfo_t &info ) { return false; }
IDirect3DDevice9* CShaderDeviceD3D8FF::InvokeCreateDevice( void* hWnd, int nAdapter, DWORD deviceCreationFlags ) { return NULL; }
void CShaderDeviceD3D8FF::SetPresentParameters( void* hWnd, int nAdapter, const ShaderDeviceInfo_t &info ) {}
D3DMULTISAMPLE_TYPE CShaderDeviceD3D8FF::ComputeMultisampleType( int nSampleCount ) { return D3DMULTISAMPLE_NONE; }
bool CShaderDeviceD3D8FF::TryDeviceReset() { return false; }
void CShaderDeviceD3D8FF::MarkDeviceLost() {}
void CShaderDeviceD3D8FF::CheckDeviceLost( bool bOtherAppInitializing ) {}
bool CShaderDeviceD3D8FF::ResizeWindow( const ShaderDeviceInfo_t &info ) { return false; }
bool CShaderDeviceD3D8FF::InNonInteractiveMode() const { return false; }
void CShaderDeviceD3D8FF::ReacquireResourcesInternal( bool bResetState, bool bForceReacquire, char const *pszForceReason ) {}

//-----------------------------------------------------------------------------
// Called when the adapter is set
//-----------------------------------------------------------------------------
bool CShaderDeviceD3D8FF::OnAdapterSet()
{
	// Nothing special needs to be done for D3D8FF
	return true;
}

//-----------------------------------------------------------------------------
// Resets the render state
//-----------------------------------------------------------------------------
void CShaderDeviceD3D8FF::ResetRenderState( bool bFullReset )
{
	// Reset D3D9 fixed function state to defaults
	if ( g_pD3DDevice )
	{
		// This will be implemented when we have a working device
		// For now, just stub it out
	}
}

