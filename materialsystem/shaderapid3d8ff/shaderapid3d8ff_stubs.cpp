//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Shader API - Stub implementations for advanced features
//
//===========================================================================//

#include "shaderapid3d8ff_impl.h"
#include "shaderdeviced3d8ff.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Advanced rendering features (not used in fixed function)
//-----------------------------------------------------------------------------

void CShaderAPID3D8FF::SetShadowDepthBiasFactors( float fShadowSlopeScaleDepthBias, float fShadowDepthBias )
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::BindVertexBuffer( int nStreamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes, int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions )
{
	// TODO: Implement if needed for advanced rendering
}

void CShaderAPID3D8FF::BindIndexBuffer( IIndexBuffer *pIndexBuffer, int nOffsetInBytes )
{
	// TODO: Implement if needed for advanced rendering
}

void CShaderAPID3D8FF::Draw( MaterialPrimitiveType_t primitiveType, int nFirstIndex, int nIndexCount )
{
	// TODO: Implement if needed for advanced rendering
}

void CShaderAPID3D8FF::PerformFullScreenStencilOperation( void )
{
	// Not commonly used
}

void CShaderAPID3D8FF::SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor )
{
	if ( !Dx9Device() )
		return;
	
	if ( bEnableScissor )
	{
		RECT rect = { nLeft, nTop, nRight, nBottom };
		Dx9Device()->SetScissorRect( &rect );
		Dx9Device()->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	}
	else
	{
		Dx9Device()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
	}
}

bool CShaderAPID3D8FF::SupportsCSAAMode( int nNumSamples, int nQualityLevel )
{
	return false; // No CSAA support in fixed function
}

void CShaderAPID3D8FF::InvalidateDelayedShaderConstants( void )
{
	// No shader constants in fixed function
}

float CShaderAPID3D8FF::GammaToLinear_HardwareSpecific( float fGamma ) const
{
	return fGamma; // No hardware gamma conversion
}

float CShaderAPID3D8FF::LinearToGamma_HardwareSpecific( float fLinear ) const
{
	return fLinear; // No hardware gamma conversion
}

void CShaderAPID3D8FF::SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture )
{
	// Not used in fixed function
}

ImageFormat CShaderAPID3D8FF::GetNullTextureFormat( void )
{
	return IMAGE_FORMAT_RGBA8888;
}

void CShaderAPID3D8FF::BindVertexTexture( VertexTextureSampler_t nSampler, ShaderAPITextureHandle_t textureHandle )
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::EnableHWMorphing( bool bEnable )
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::SetFlexWeights( int nFirstWeight, int nCount, const MorphWeight_t* pWeights )
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::FogMaxDensity( float flMaxDensity )
{
	// Fixed function fog doesn't use max density
}

void CShaderAPID3D8FF::CreateTextures( ShaderAPITextureHandle_t *pHandles, int count, int width, int height, int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags, const char *pDebugName, const char *pTextureGroupName )
{
	// Batch create - call individual CreateTexture for each
	for ( int i = 0; i < count; ++i )
	{
		pHandles[i] = CreateTexture( width, height, depth, dstImageFormat, numMipLevels, numCopies, flags, pDebugName, pTextureGroupName );
	}
}

void CShaderAPID3D8FF::AcquireThreadOwnership()
{
	// Single-threaded rendering
}

void CShaderAPID3D8FF::ReleaseThreadOwnership()
{
	// Single-threaded rendering
}

bool CShaderAPID3D8FF::SupportsNormalMapCompression() const
{
	return false; // No compressed normal maps in fixed function
}

void CShaderAPID3D8FF::EnableBuffer2FramesAhead( bool bEnable )
{
	// Not relevant for fixed function
}

void CShaderAPID3D8FF::SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale )
{
	// No pixel shaders in fixed function
}

void CShaderAPID3D8FF::OverrideAlphaWriteEnable( bool bEnable, bool bAlphaWriteEnable )
{
	if ( !Dx9Device() )
		return;
	
	if ( bEnable )
	{
		Dx9Device()->SetRenderState( D3DRS_COLORWRITEENABLE, bAlphaWriteEnable ? D3DCOLORWRITEENABLE_ALL : (D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE) );
	}
}

void CShaderAPID3D8FF::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
	if ( !Dx9Device() )
		return;
	
	if ( bOverrideEnable )
	{
		Dx9Device()->SetRenderState( D3DRS_COLORWRITEENABLE, bColorWriteEnable ? D3DCOLORWRITEENABLE_ALL : 0 );
	}
}

void CShaderAPID3D8FF::ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth )
{
	ClearBuffers( bClearColor, bClearDepth, false, 0, 0 );
}

void CShaderAPID3D8FF::CopyRenderTargetToScratchTexture( ShaderAPITextureHandle_t srcRt, ShaderAPITextureHandle_t dstTex, Rect_t *pSrcRect, Rect_t *pDstRect )
{
	// Advanced feature not commonly used
}

void CShaderAPID3D8FF::LockRect( void** pOutBits, int* pOutPitch, ShaderAPITextureHandle_t texHandle, int mipmap, int x, int y, int w, int h, bool bWrite, bool bRead )
{
	// Use TexLock instead
}

void CShaderAPID3D8FF::UnlockRect( ShaderAPITextureHandle_t texHandle, int mipmap )
{
	// Use TexUnlock instead
}

void CShaderAPID3D8FF::TexLodClamp( int finest )
{
	// Not commonly used
}

void CShaderAPID3D8FF::TexLodBias( float bias )
{
	// Not commonly used
}

void CShaderAPID3D8FF::CopyTextureToTexture( ShaderAPITextureHandle_t srcTex, ShaderAPITextureHandle_t dstTex )
{
	// Advanced feature
}

void CShaderAPID3D8FF::EnableAlphaToCoverage()
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::DisableAlphaToCoverage()
{
	// Not supported in fixed function
}

void CShaderAPID3D8FF::ComputeVertexDescription( unsigned char* pBuffer, VertexFormat_t vertexFormat, MeshDesc_t& desc ) const
{
	// Vertex format description - can implement if needed
}

bool CShaderAPID3D8FF::SupportsShadowDepthTextures( void )
{
	return false;
}

void CShaderAPID3D8FF::SetDisallowAccess( bool bDisallow )
{
	// Thread safety - not needed for single-threaded
}

void CShaderAPID3D8FF::EnableShaderShaderMutex( bool bEnable )
{
	// Thread safety - not needed for single-threaded
}

void CShaderAPID3D8FF::ShaderLock()
{
	// Thread safety - not needed for single-threaded
}

void CShaderAPID3D8FF::ShaderUnlock()
{
	// Thread safety - not needed for single-threaded
}

ImageFormat CShaderAPID3D8FF::GetShadowDepthTextureFormat( void )
{
	return IMAGE_FORMAT_UNKNOWN;
}

bool CShaderAPID3D8FF::SupportsFetch4( void )
{
	return false;
}

IMesh *CShaderAPID3D8FF::GetFlexMesh()
{
	return NULL; // No morph/flex support in fixed function
}

void CShaderAPID3D8FF::SetFlashlightStateEx( const FlashlightState_t &state, const VMatrix &worldToTexture, ITexture *pFlashlightDepthTexture )
{
	// Flashlight not supported in fixed function
}

bool CShaderAPID3D8FF::SupportsMSAAMode( int nMSAAMode )
{
	return false;
}

bool CShaderAPID3D8FF::OwnGPUResources( bool bEnable )
{
	return true; // Always own resources
}

void CShaderAPID3D8FF::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
	if ( fStart )
		*fStart = 0.0f;
	if ( fEnd )
		*fEnd = 1.0f;
	if ( fFogZ )
		*fFogZ = 0.0f;
}

void CShaderAPID3D8FF::SetPIXMarker( unsigned long color, const char *szName )
{
	// PIX markers for debugging - not critical
}

void CShaderAPID3D8FF::DisableAllLocalLights()
{
	if ( !Dx9Device() )
		return;
	
	// Disable all D3D lights
	for ( int i = 0; i < D3D8FF_MAX_LIGHTS; ++i )
	{
		Dx9Device()->LightEnable( i, FALSE );
	}
}

int CShaderAPID3D8FF::CompareSnapshots( StateSnapshot_t snapshot0, StateSnapshot_t snapshot1 )
{
	// Simple comparison - could be implemented if needed for optimization
	return (snapshot0 == snapshot1) ? 0 : 1;
}

void CShaderAPID3D8FF::GetMaxToRender( IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices )
{
	if ( pMaxVerts )
		*pMaxVerts = 65535;
	if ( pMaxIndices )
		*pMaxIndices = 65535;
}

int CShaderAPID3D8FF::GetMaxVerticesToRender( IMaterial *pMaterial )
{
	return 65535;
}

int CShaderAPID3D8FF::GetMaxIndicesToRender()
{
	return 65535;
}

void CShaderAPID3D8FF::UserClipTransform( const VMatrix &worldToView )
{
	// Clip plane transformation
}

MorphFormat_t CShaderAPID3D8FF::ComputeMorphFormat( int numSnapshots, StateSnapshot_t* pIds ) const
{
	return 0; // No morphing in fixed function
}

void CShaderAPID3D8FF::SetRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
{
	// Extended render target - use basic SetRenderTarget
}

void CShaderAPID3D8FF::CopyRenderTargetToTextureEx( ShaderAPITextureHandle_t textureHandle, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect )
{
	// Extended copy - use basic CopyRenderTargetToTexture
}

void CShaderAPID3D8FF::CopyTextureToRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t textureHandle, Rect_t *pSrcRect, Rect_t *pDstRect )
{
	// Not commonly used
}

void CShaderAPID3D8FF::HandleDeviceLost()
{
	// Device lost handling
}

void CShaderAPID3D8FF::EnableLinearColorSpaceFrameBuffer( bool bEnable )
{
	// sRGB framebuffer - not supported in fixed function
}

void CShaderAPID3D8FF::SetFullScreenTextureHandle( ShaderAPITextureHandle_t h )
{
	// Full screen texture for effects
}

void CShaderAPID3D8FF::SetFastClipPlane( const float *pPlane )
{
	// Fast clip plane
}

void CShaderAPID3D8FF::EnableFastClip( bool bEnable )
{
	// Fast clip enable/disable
}

void CShaderAPID3D8FF::EnableUserClipTransformOverride( bool bEnable )
{
	// User clip transform override
}

int CShaderAPID3D8FF::OcclusionQuery_GetNumPixelsRendered( ShaderAPIOcclusionQuery_t hQuery, bool bFlush )
{
	return 0; // Occlusion queries not supported
}

void CShaderAPID3D8FF::SetFlashlightState( const FlashlightState_t &state, const VMatrix &worldToTexture )
{
	// Flashlight not supported in fixed function
}

void CShaderAPID3D8FF::ClearVertexAndPixelShaderRefCounts()
{
	// No shaders in fixed function
}

void CShaderAPID3D8FF::PurgeUnusedVertexAndPixelShaders()
{
	// No shaders in fixed function
}

void CShaderAPID3D8FF::DXSupportLevelChanged()
{
	// DX level doesn't change in fixed function
}

void CShaderAPID3D8FF::EvictManagedResources()
{
	if ( Dx9Device() )
		Dx9Device()->EvictManagedResources();
}

void CShaderAPID3D8FF::SetAnisotropicLevel( int nAnisotropyLevel )
{
	// Can be implemented if needed
}

void CShaderAPID3D8FF::SyncToken( const char *pToken )
{
	// Synchronization token - not commonly used
}

void CShaderAPID3D8FF::SetStandardVertexShaderConstants( float fOverbright )
{
	// No vertex shaders in fixed function
}

ShaderAPIOcclusionQuery_t CShaderAPID3D8FF::CreateOcclusionQueryObject( void )
{
	return 0; // No occlusion queries
}

void CShaderAPID3D8FF::DestroyOcclusionQueryObject( ShaderAPIOcclusionQuery_t hQuery )
{
	// No occlusion queries
}

void CShaderAPID3D8FF::BeginOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t hQuery )
{
	// No occlusion queries
}

void CShaderAPID3D8FF::EndOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t hQuery )
{
	// No occlusion queries
}

int CShaderAPID3D8FF::SelectionMode( bool selectionMode )
{
	return 0; // Selection mode not supported
}

void CShaderAPID3D8FF::SelectionBuffer( unsigned int* pBuffer, int size )
{
	// Selection mode not supported
}

void CShaderAPID3D8FF::ClearSelectionNames()
{
	// Selection mode not supported
}

void CShaderAPID3D8FF::LoadSelectionName( int name )
{
	// Selection mode not supported
}

void CShaderAPID3D8FF::PushSelectionName( int name )
{
	// Selection mode not supported
}

void CShaderAPID3D8FF::PopSelectionName()
{
	// Selection mode not supported
}

void CShaderAPID3D8FF::ForceHardwareSync()
{
	// Hardware sync
}

void CShaderAPID3D8FF::ClearSnapshots()
{
	// Clear snapshot list
}

void CShaderAPID3D8FF::FogStart( float fStart )
{
	// Set fog start distance
}

void CShaderAPID3D8FF::FogEnd( float fEnd )
{
	// Set fog end distance
}

void CShaderAPID3D8FF::SetFogZ( float fogZ )
{
	// Set fog Z value
}

void CShaderAPID3D8FF::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	// Set scene fog color
}

void CShaderAPID3D8FF::GetSceneFogColor( unsigned char *rgb )
{
	if ( rgb )
	{
		rgb[0] = rgb[1] = rgb[2] = 128;
	}
}

void CShaderAPID3D8FF::SceneFogMode( MaterialFogMode_t fogMode )
{
	// Set scene fog mode
}

bool CShaderAPID3D8FF::CanDownloadTextures() const
{
	return true;
}

void CShaderAPID3D8FF::ResetRenderState( bool bFullReset )
{
	// Reset render state
}

int CShaderAPID3D8FF::GetCurrentDynamicVBSize( void )
{
	return 0;
}

void CShaderAPID3D8FF::DestroyVertexBuffers( bool bExitingLevel )
{
	// Destroy vertex buffers
}

void CShaderAPID3D8FF::ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat )
{
	// Read pixels from backbuffer
}

void CShaderAPID3D8FF::ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride )
{
	// Read pixels with rectangles
}

void CShaderAPID3D8FF::FlushHardware()
{
	// Flush hardware
}

void CShaderAPID3D8FF::BeginFrame()
{
	if ( Dx9Device() )
		Dx9Device()->BeginScene();
}

void CShaderAPID3D8FF::EndFrame()
{
	if ( Dx9Device() )
		Dx9Device()->EndScene();
}

void CShaderAPID3D8FF::SetRenderTarget( ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
{
	// Set render target
}

void CShaderAPID3D8FF::ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth )
{
	ClearBuffers( bClearColor, bClearDepth, false, 0, 0 );
}

void CShaderAPID3D8FF::TexSetPriority( int priority )
{
	// Texture priority
}

void CShaderAPID3D8FF::BindTexture( Sampler_t sampler, ShaderAPITextureHandle_t textureHandle )
{
	// Bind texture to sampler
}

// Shader constants (not used in fixed function)
void CShaderAPID3D8FF::SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numBools, bool bForce )
{
}

void CShaderAPID3D8FF::SetIntegerVertexShaderConstant( int var, int const* pVec, int numIntVecs, bool bForce )
{
}

void CShaderAPID3D8FF::SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools, bool bForce )
{
}

void CShaderAPID3D8FF::SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs, bool bForce )
{
}

bool CShaderAPID3D8FF::ShouldWriteDepthToDestAlpha( void ) const
{
	return false;
}

void CShaderAPID3D8FF::PushDeformation( DeformationBase_t const *Deformation )
{
}

void CShaderAPID3D8FF::PopDeformation()
{
}

void CShaderAPID3D8FF::MarkUnusedVertexFields( unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords )
{
}

void CShaderAPID3D8FF::SetPSNearAndFarZ( int pshReg )
{
}

// Texture management
void CShaderAPID3D8FF::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
}

bool CShaderAPID3D8FF::IsTexture( ShaderAPITextureHandle_t textureHandle )
{
	return false;
}

bool CShaderAPID3D8FF::IsTextureResident( ShaderAPITextureHandle_t textureHandle )
{
	return true;
}

void CShaderAPID3D8FF::ModifyTexture( ShaderAPITextureHandle_t textureHandle )
{
}

void CShaderAPID3D8FF::TexImageFromVTF( IVTFTexture* pVTF, int iVTFFrame )
{
}

bool CShaderAPID3D8FF::TexLock( int level, int cubeFaceID, int xOffset, int yOffset, int width, int height, CPixelWriter& writer )
{
	return false;
}

void CShaderAPID3D8FF::TexUnlock()
{
}

