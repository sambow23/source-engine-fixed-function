//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Shader API Core Implementation
//
//===========================================================================//

#include "shaderapid3d8ff_impl.h"
#include "shaderdeviced3d8ff.h"
#include "hardwareconfig.h"
#include "mesh_d3d8ff.h"
#include "tier0/dbg.h"
#include "tier0/vprof.h"
#include "tier1/strtools.h"
#include "materialsystem/imaterialsystem.h"
#include "imaterialinternal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
static CShaderAPID3D8FF s_ShaderAPID3D8FF;
CShaderAPID3D8FF *g_pShaderAPID3D8FF = &s_ShaderAPID3D8FF;

CShaderAPID3D8FF* ShaderAPI()
{
	return &s_ShaderAPID3D8FF;
}

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
CShaderAPID3D8FF::CShaderAPID3D8FF()
{
	m_pBoundMaterial = NULL;
	m_bRenderingMesh = false;
	m_nFrameCounter = 0;
	m_nCurrentSnapshot = -1;
	m_bLightsDirty = false;
	
	memset( &m_DynamicState, 0, sizeof(m_DynamicState) );
	memset( &m_CurrentShadowState, 0, sizeof(m_CurrentShadowState) );
	memset( m_Lights, 0, sizeof(m_Lights) );
	memset( m_bLightEnabled, 0, sizeof(m_bLightEnabled) );
	
	m_DynamicState.m_CullMode = D3DCULL_CCW;
	m_DynamicState.m_ClearColor = D3DCOLOR_XRGB(0, 0, 0);
	m_DynamicState.m_bClearColorDirty = true;
}

CShaderAPID3D8FF::~CShaderAPID3D8FF()
{
}

//-----------------------------------------------------------------------------
// Device initialization/shutdown
//-----------------------------------------------------------------------------
bool CShaderAPID3D8FF::OnDeviceInit()
{
	Msg( "D3D8FF: Shader API initialized\n" );
	
	// Initialize mesh manager
	g_MeshMgr.Init();
	
	// Initialize all render states to defaults
	ResetRenderState( true );
	
	// Make ourselves the global shader API
	g_pShaderAPI = this;
	
	return true;
}

void CShaderAPID3D8FF::OnDeviceShutdown()
{
	// Shutdown mesh manager
	g_MeshMgr.Shutdown();
	
	g_pShaderAPI = NULL;
}

//-----------------------------------------------------------------------------
// PIX events
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::BeginPIXEvent( unsigned long color, const char *szName )
{
#ifdef _WIN32
	// D3DPERF_BeginEvent( color, szName );
#endif
}

void CShaderAPID3D8FF::EndPIXEvent()
{
#ifdef _WIN32
	// D3DPERF_EndEvent();
#endif
}

void CShaderAPID3D8FF::AdvancePIXFrame()
{
	m_nFrameCounter++;
}

//-----------------------------------------------------------------------------
// Resource management
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ReleaseShaderObjects()
{
	// Fixed function doesn't have shader objects to release
}

void CShaderAPID3D8FF::RestoreShaderObjects()
{
	// Fixed function doesn't have shader objects to restore
}

//-----------------------------------------------------------------------------
// Reset render state
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ResetRenderState( bool bFullReset )
{
	if ( !Dx9Device() )
		return;
	
	Msg( "D3D8FF: Resetting render state (full=%d)\n", bFullReset );
	
	// Reset current snapshot
	m_nCurrentSnapshot = -1;
	
	// Reset clear color
	m_DynamicState.m_ClearColor = D3DCOLOR_XRGB(0, 0, 0);
	m_DynamicState.m_bClearColorDirty = true;
	
	// Reset current shadow state
	memset( &m_CurrentShadowState, 0, sizeof(m_CurrentShadowState) );
	m_CurrentShadowState.m_bDirty = true;
	m_CurrentShadowState.m_CullMode = D3DCULL_CCW;
	m_CurrentShadowState.m_bZEnable = true;
	m_CurrentShadowState.m_bZWriteEnable = true;
	m_CurrentShadowState.m_ZFunc = D3DCMP_LESSEQUAL;
	
	// Initialize all texture stages to disabled
	for ( int i = 0; i < D3D8FF_MAX_TEXTURE_STAGES; ++i )
	{
		m_CurrentShadowState.m_TextureStage[i].m_bEnabled = false;
		m_CurrentShadowState.m_TextureStage[i].m_ColorOp = D3DTOP_DISABLE;
		m_CurrentShadowState.m_TextureStage[i].m_AlphaOp = D3DTOP_DISABLE;
		m_CurrentShadowState.m_TextureStage[i].m_ColorArg1 = D3DTA_TEXTURE;
		m_CurrentShadowState.m_TextureStage[i].m_ColorArg2 = D3DTA_CURRENT;
		m_CurrentShadowState.m_TextureStage[i].m_AlphaArg1 = D3DTA_TEXTURE;
		m_CurrentShadowState.m_TextureStage[i].m_AlphaArg2 = D3DTA_CURRENT;
		m_CurrentShadowState.m_TextureStage[i].m_TexCoordIndex = i;
		m_CurrentShadowState.m_TextureStage[i].m_TexTransformFlags = D3DTTFF_DISABLE;
	}
	
	// Set stage 0 to modulate by default
	m_CurrentShadowState.m_TextureStage[0].m_bEnabled = true;
	m_CurrentShadowState.m_TextureStage[0].m_ColorOp = D3DTOP_MODULATE;
	m_CurrentShadowState.m_TextureStage[0].m_AlphaOp = D3DTOP_SELECTARG1;
	
	// Disable all lights
	for ( int i = 0; i < D3D8FF_MAX_LIGHTS; ++i )
	{
		m_bLightEnabled[i] = false;
	}
	m_bLightsDirty = true;
	
	// Apply all states to device
	CommitStateChanges();
}

//-----------------------------------------------------------------------------
// Texture access
//-----------------------------------------------------------------------------
IDirect3DBaseTexture* CShaderAPID3D8FF::GetD3DTexture( ShaderAPITextureHandle_t hTexture )
{
	// To be implemented when texture system is added
	return NULL;
}

void CShaderAPID3D8FF::QueueResetRenderState()
{
	// Queue a reset for next frame
	m_CurrentShadowState.m_bDirty = true;
}

//-----------------------------------------------------------------------------
// Viewport
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetViewports( int nCount, const ShaderViewport_t* pViewports )
{
	if ( !Dx9Device() || nCount == 0 || !pViewports )
		return;
	
	// D3D8/9 only supports one viewport
	D3DVIEWPORT9 viewport;
	viewport.X = pViewports[0].m_nTopLeftX;
	viewport.Y = pViewports[0].m_nTopLeftY;
	viewport.Width = pViewports[0].m_nWidth;
	viewport.Height = pViewports[0].m_nHeight;
	viewport.MinZ = pViewports[0].m_flMinZ;
	viewport.MaxZ = pViewports[0].m_flMaxZ;
	
	Dx9Device()->SetViewport( &viewport );
	
	m_DynamicState.m_Viewport = viewport;
}

int CShaderAPID3D8FF::GetViewports( ShaderViewport_t* pViewports, int nMax ) const
{
	if ( !pViewports || nMax == 0 )
		return 0;
	
	pViewports[0].m_nTopLeftX = m_DynamicState.m_Viewport.X;
	pViewports[0].m_nTopLeftY = m_DynamicState.m_Viewport.Y;
	pViewports[0].m_nWidth = m_DynamicState.m_Viewport.Width;
	pViewports[0].m_nHeight = m_DynamicState.m_Viewport.Height;
	pViewports[0].m_flMinZ = m_DynamicState.m_Viewport.MinZ;
	pViewports[0].m_flMaxZ = m_DynamicState.m_Viewport.MaxZ;
	pViewports[0].m_nVersion = SHADER_VIEWPORT_VERSION;
	
	return 1;
}

//-----------------------------------------------------------------------------
// Buffer clearing
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil, 
									  int renderTargetWidth, int renderTargetHeight )
{
	if ( !Dx9Device() )
		return;
	
	DWORD dwFlags = 0;
	if ( bClearColor )
		dwFlags |= D3DCLEAR_TARGET;
	if ( bClearDepth )
		dwFlags |= D3DCLEAR_ZBUFFER;
	if ( bClearStencil )
		dwFlags |= D3DCLEAR_STENCIL;
	
	if ( dwFlags != 0 )
	{
		Dx9Device()->Clear( 0, NULL, dwFlags, m_DynamicState.m_ClearColor, 1.0f, 0 );
	}
}

void CShaderAPID3D8FF::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	m_DynamicState.m_ClearColor = D3DCOLOR_XRGB( r, g, b );
	m_DynamicState.m_bClearColorDirty = true;
}

void CShaderAPID3D8FF::ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	m_DynamicState.m_ClearColor = D3DCOLOR_ARGB( a, r, g, b );
	m_DynamicState.m_bClearColorDirty = true;
}

//-----------------------------------------------------------------------------
// Rasterizer state
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetRasterState( const ShaderRasterState_t& state )
{
	if ( !Dx9Device() )
		return;
	
	// Set fill mode
	D3DFILLMODE fillMode = ( state.m_FillMode == SHADER_FILL_WIREFRAME ) ? D3DFILL_WIREFRAME : D3DFILL_SOLID;
	Dx9Device()->SetRenderState( D3DRS_FILLMODE, fillMode );
	
	// Set cull mode
	if ( state.m_bCullEnable )
	{
		D3DCULL cullMode = D3DCULL_NONE;
		switch ( state.m_CullMode )
		{
		case MATERIAL_CULLMODE_CCW:
			cullMode = D3DCULL_CCW;
			break;
		case MATERIAL_CULLMODE_CW:
			cullMode = D3DCULL_CW;
			break;
		}
		Dx9Device()->SetRenderState( D3DRS_CULLMODE, cullMode );
		m_DynamicState.m_CullMode = cullMode;
	}
	else
	{
		Dx9Device()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		m_DynamicState.m_CullMode = D3DCULL_NONE;
	}
	
	// Scissor test
	Dx9Device()->SetRenderState( D3DRS_SCISSORTESTENABLE, state.m_bScissorEnable );
}

//-----------------------------------------------------------------------------
// Snapshot management
//-----------------------------------------------------------------------------
StateSnapshot_t CShaderAPID3D8FF::TakeSnapshot()
{
	// Create a copy of the current shadow state
	ShadowState_t snapshot = m_CurrentShadowState;
	snapshot.m_bDirty = false;
	
	// Add to list
	int id = m_SnapshotList.AddToTail( snapshot );
	
	return (StateSnapshot_t)id;
}

//-----------------------------------------------------------------------------
// Texture filtering
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::TexMinFilter( ShaderTexFilterMode_t texFilterMode )
{
	// To be implemented when texture system is ready
}

void CShaderAPID3D8FF::TexMagFilter( ShaderTexFilterMode_t texFilterMode )
{
	// To be implemented when texture system is ready
}

void CShaderAPID3D8FF::TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode )
{
	// To be implemented when texture system is ready
}

//-----------------------------------------------------------------------------
// Copy render target
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::CopyRenderTargetToTexture( ShaderAPITextureHandle_t textureHandle )
{
	// To be implemented when texture system is ready
}

//-----------------------------------------------------------------------------
// Material binding
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::Bind( IMaterial* pMaterial )
{
	m_pBoundMaterial = static_cast<IMaterialInternal*>( pMaterial );
}

//-----------------------------------------------------------------------------
// Flush buffered primitives
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::FlushBufferedPrimitives()
{
	g_MeshMgr.Flush();
}

//-----------------------------------------------------------------------------
// Dynamic mesh access
//-----------------------------------------------------------------------------
IMesh* CShaderAPID3D8FF::GetDynamicMesh( IMaterial* pMaterial, int nHWSkinBoneCount, bool bBuffered,
	IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	VertexFormat_t vertexFormat = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2);
	return g_MeshMgr.GetDynamicMesh( pMaterial, vertexFormat, nHWSkinBoneCount, bBuffered, pVertexOverride, pIndexOverride );
}

IMesh* CShaderAPID3D8FF::GetDynamicMeshEx( IMaterial* pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount, 
	bool bBuffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	return g_MeshMgr.GetDynamicMesh( pMaterial, vertexFormat, nHWSkinBoneCount, bBuffered, pVertexOverride, pIndexOverride );
}

//-----------------------------------------------------------------------------
// Snapshot queries
//-----------------------------------------------------------------------------
bool CShaderAPID3D8FF::IsTranslucent( StateSnapshot_t id ) const
{
	if ( id < 0 || id >= m_SnapshotList.Count() )
		return false;
	
	return m_SnapshotList[id].m_bAlphaBlendEnable;
}

bool CShaderAPID3D8FF::IsAlphaTested( StateSnapshot_t id ) const
{
	// Alpha test not commonly used in fixed function
	return false;
}

bool CShaderAPID3D8FF::IsDepthWriteEnabled( StateSnapshot_t id ) const
{
	if ( id < 0 || id >= m_SnapshotList.Count() )
		return true;
	
	return m_SnapshotList[id].m_bZWriteEnable;
}

//-----------------------------------------------------------------------------
// Vertex format
//-----------------------------------------------------------------------------
VertexFormat_t CShaderAPID3D8FF::ComputeVertexFormat( int numSnapshots, StateSnapshot_t* pIds ) const
{
	// Basic fixed function format: position, normal, color, texcoord
	return VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2);
}

VertexFormat_t CShaderAPID3D8FF::ComputeVertexUsage( int numSnapshots, StateSnapshot_t* pIds ) const
{
	return ComputeVertexFormat( numSnapshots, pIds );
}

//-----------------------------------------------------------------------------
// Rendering pass
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::BeginPass( StateSnapshot_t snapshot )
{
	VPROF( "CShaderAPID3D8FF::BeginPass" );
	
	m_nCurrentSnapshot = snapshot;
	
	if ( snapshot >= 0 && snapshot < m_SnapshotList.Count() )
	{
		// Apply the snapshot state
		m_CurrentShadowState = m_SnapshotList[snapshot];
		m_CurrentShadowState.m_bDirty = true;
	}
}

void CShaderAPID3D8FF::RenderPass( int nPass, int nPassCount )
{
	VPROF( "CShaderAPID3D8FF::RenderPass" );
	
	if ( m_nCurrentSnapshot == -1 )
		return;
	
	// Commit all state changes
	CommitStateChanges();
	
	// Mesh will actually draw in its RenderPass() method
	m_nCurrentSnapshot = -1;
}

//-----------------------------------------------------------------------------
// Bone weights
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetNumBoneWeights( int numBones )
{
	// Fixed function skinning not supported yet
}

//-----------------------------------------------------------------------------
// Print functions
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::PrintfVA( char *fmt, va_list vargs )
{
	char buf[2048];
	V_vsnprintf( buf, sizeof(buf), fmt, vargs );
	Msg( "%s", buf );
}

void CShaderAPID3D8FF::Printf( PRINTF_FORMAT_STRING const char *fmt, ... )
{
	va_list vargs;
	va_start( vargs, fmt );
	PrintfVA( (char*)fmt, vargs );
	va_end( vargs );
}

//-----------------------------------------------------------------------------
// Software vertex processing query
//-----------------------------------------------------------------------------
bool CShaderAPID3D8FF::UsingSoftwareVertexProcessing() const
{
	return HardwareConfig()->GetDXSupportLevel() < 90;
}

// Continued in next file...

