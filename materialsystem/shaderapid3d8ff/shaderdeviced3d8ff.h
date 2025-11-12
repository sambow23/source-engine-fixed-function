//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8 Fixed Function Shader Device
//
//===========================================================================//

#ifndef SHADERDEVICED3D8FF_H
#define SHADERDEVICED3D8FF_H

#ifdef _WIN32
#pragma once
#endif

#include "../shaderapidx9/shaderdevicebase.h"
#include "shaderapid3d8ff_global.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"

//-----------------------------------------------------------------------------
// Device type - use D3D9 HAL for compatibility
//-----------------------------------------------------------------------------
#ifndef USE_REFERENCE_RASTERIZER
#define D3D8FF_DEVTYPE D3DDEVTYPE_HAL
#else
#define D3D8FF_DEVTYPE D3DDEVTYPE_REF
#endif

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct MaterialAdapterInfo_t;
class KeyValues;

//-----------------------------------------------------------------------------
// The D3D8FF Shader Device Manager
//-----------------------------------------------------------------------------
class CShaderDeviceMgrD3D8FF : public CShaderDeviceMgrBase
{
	typedef CShaderDeviceMgrBase BaseClass;

public:
	// Constructor, destructor
	CShaderDeviceMgrD3D8FF();
	virtual ~CShaderDeviceMgrD3D8FF();

	// Methods of IAppSystem
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();
	virtual InitReturnVal_t Init();
	virtual void Shutdown();

	// Methods of IShaderDeviceMgr
	virtual int GetAdapterCount() const;
	virtual void GetAdapterInfo( int adapter, MaterialAdapterInfo_t& info ) const;
	virtual int GetModeCount( int nAdapter ) const;
	virtual void GetModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter, int mode ) const;
	virtual void GetCurrentModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter ) const;
	virtual bool SetAdapter( int nAdapter, int nFlags );
	virtual CreateInterfaceFn SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t& mode );

	// Determines hardware caps from D3D
	bool ComputeCapsFromD3D( HardwareCaps_t *pCaps, int nAdapter );

	// Validates the mode
	bool ValidateMode( int nAdapter, const ShaderDeviceInfo_t &info ) const;

	// Returns the amount of video memory in bytes for a particular adapter
	virtual int GetVidMemBytes( int nAdapter ) const;

	FORCEINLINE IDirect3D9 *D3D() const
	{ 
		return m_pD3D; 
	}

protected:
	// Determine capabilities
	bool DetermineHardwareCaps();

private:
	// Initialize adapter information
	void InitAdapterInfo();

	// Compute the effective DX support level based on caps
	void ComputeDXSupportLevel( HardwareCaps_t &caps );

	// Used to enumerate adapters, attach to windows
	IDirect3D9 *m_pD3D;
	bool m_bAdapterInfoInitialized : 1;
};

extern CShaderDeviceMgrD3D8FF* g_pShaderDeviceMgrD3D8FF;

//-----------------------------------------------------------------------------
// IDirect3D accessor
//-----------------------------------------------------------------------------
inline IDirect3D9* D3D()  
{
	return g_pShaderDeviceMgrD3D8FF->D3D();
}

//-----------------------------------------------------------------------------
// The D3D8FF Shader Device
//-----------------------------------------------------------------------------
class CShaderDeviceD3D8FF : public CShaderDeviceBase
{
	// Methods of IShaderDevice
public:
	virtual bool IsUsingGraphics() const;
	virtual ImageFormat GetBackBufferFormat() const;
	virtual void GetBackBufferDimensions( int& width, int& height ) const;
	virtual void Present();
	virtual IShaderBuffer* CompileShader( const char *pProgram, size_t nBufLen, const char *pShaderVersion );
	virtual VertexShaderHandle_t CreateVertexShader( IShaderBuffer *pBuffer );
	virtual void DestroyVertexShader( VertexShaderHandle_t hShader );
	virtual GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer* pShaderBuffer );
	virtual void DestroyGeometryShader( GeometryShaderHandle_t hShader );
	virtual PixelShaderHandle_t CreatePixelShader( IShaderBuffer* pShaderBuffer );
	virtual void DestroyPixelShader( PixelShaderHandle_t hShader );
	virtual void ReleaseResources();
	virtual void ReacquireResources();
	virtual IMesh* CreateStaticMesh( VertexFormat_t format, const char *pBudgetGroup, IMaterial * pMaterial = NULL );
	virtual void DestroyStaticMesh( IMesh* mesh );
	virtual IVertexBuffer *CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup );
	virtual void DestroyVertexBuffer( IVertexBuffer *pVertexBuffer );
	virtual IIndexBuffer *CreateIndexBuffer( ShaderBufferType_t bufferType, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup );
	virtual void DestroyIndexBuffer( IIndexBuffer *pIndexBuffer );
	virtual IVertexBuffer *GetDynamicVertexBuffer( int nStreamID, VertexFormat_t vertexFormat, bool bBuffered = true );
	virtual IIndexBuffer *GetDynamicIndexBuffer( MaterialIndexFormat_t fmt, bool bBuffered = true );
	virtual void SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax, float fGammaTVExponent, bool bTVEnabled );
	virtual void SpewDriverInfo() const;
	virtual int GetCurrentAdapter() const;
	virtual void EnableNonInteractiveMode( MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo = NULL );
	virtual void RefreshFrontBufferNonInteractive();
	virtual char *GetDisplayDeviceName() OVERRIDE;

	// Methods of CShaderDeviceBase
public:
	virtual bool InitDevice( void* hWnd, int nAdapter, const ShaderDeviceInfo_t &info );
	virtual void ShutdownDevice();
	virtual bool IsDeactivated() const;

	// Other public methods
public:
	// Constructor, destructor
	CShaderDeviceD3D8FF();
	virtual ~CShaderDeviceD3D8FF();

	// Call this when another app is initializing
	virtual void OtherAppInitializing( bool initializing );
	
	// This handles any events queued because they were called outside of the owning thread
	virtual void HandleThreadEvent( uint32 threadEvent );

	// Which device are we using?
	UINT m_DisplayAdapter;
	D3DDEVTYPE m_DeviceType;

protected:
	enum DeviceState_t
	{
		DEVICE_STATE_OK = 0,
		DEVICE_STATE_OTHER_APP_INIT,
		DEVICE_STATE_LOST_DEVICE,
		DEVICE_STATE_NEEDS_RESET,
	};

	// Creates the D3D Device
	bool CreateD3DDevice( void* pHWnd, int nAdapter, const ShaderDeviceInfo_t &info );

	// Actually creates the D3D Device once the present parameters are set up
	IDirect3DDevice9* InvokeCreateDevice( void* hWnd, int nAdapter, DWORD deviceCreationFlags );

	// Computes the presentation parameters
	void SetPresentParameters( void* hWnd, int nAdapter, const ShaderDeviceInfo_t &info );

	// Computes the supersample flags
	D3DMULTISAMPLE_TYPE ComputeMultisampleType( int nSampleCount );

	// Is the device active?
	bool IsActive() const;

	// Try to reset the device
	bool TryDeviceReset();

	// Queue up the fact that the device was lost
	void MarkDeviceLost();

	// Deals with lost devices
	void CheckDeviceLost( bool bOtherAppInitializing );

	// Changes the window size
	bool ResizeWindow( const ShaderDeviceInfo_t &info );

	// FIXME: This is for backward compat
	virtual bool OnAdapterSet();
	virtual void ResetRenderState( bool bFullReset = true );

	bool InNonInteractiveMode() const;
	void ReacquireResourcesInternal( bool bResetState = false, bool bForceReacquire = false, char const *pszForceReason = NULL );

	D3DPRESENT_PARAMETERS m_PresentParameters;
	ImageFormat m_AdapterFormat;

	// Mode info
	ShaderDeviceInfo_t m_PendingVideoModeChangeConfig;
	DeviceState_t m_DeviceState;

	bool m_bOtherAppInitializing : 1;
	bool m_bQueuedDeviceLost : 1;
	bool m_IsResizing : 1;
	bool m_bPendingVideoModeChange : 1;
	bool m_bUsingStencil : 1;
	bool m_bResourcesReleased : 1;

	// Amount of stencil variation we have available
	int m_iStencilBufferBits;

	CUtlString m_sDisplayDeviceName;

	int m_numReleaseResourcesRefCount;
};

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
extern IDirect3DDevice9 *g_pD3DDevice;

FORCEINLINE IDirect3DDevice9 *Dx9Device()
{
	return g_pD3DDevice;
}

extern CShaderDeviceD3D8FF* g_pShaderDeviceD3D8FF;

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
FORCEINLINE bool CShaderDeviceD3D8FF::IsActive() const
{
	return ( g_pD3DDevice != NULL );
}

FORCEINLINE bool CShaderDeviceD3D8FF::IsDeactivated() const 
{ 
	return ( ( m_DeviceState != DEVICE_STATE_OK ) || m_bQueuedDeviceLost || m_numReleaseResourcesRefCount ); 
}

#endif // SHADERDEVICED3D8FF_H

