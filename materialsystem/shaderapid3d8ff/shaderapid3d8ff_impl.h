//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Shader API Implementation
//
//===========================================================================//

#ifndef SHADERAPID3D8FF_IMPL_H
#define SHADERAPID3D8FF_IMPL_H

#ifdef _WIN32
#pragma once
#endif

#include "shaderapid3d8ff.h"
#include "shaderdeviced3d8ff.h"
#include "locald3dtypes.h"
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CMeshDx8;

//-----------------------------------------------------------------------------
// Transition table for state snapshots
//-----------------------------------------------------------------------------
struct ShadowState_t
{
	// Texture stages
	D3D8FFTextureStageState_t m_TextureStage[D3D8FF_MAX_TEXTURE_STAGES];
	
	// Fixed function material
	D3D8FFMaterialState_t m_Material;
	
	// Render states
	bool m_bLightingEnabled;
	bool m_bFogEnabled;
	D3DFOGMODE m_FogMode;
	D3DCOLOR m_FogColor;
	float m_FogStart;
	float m_FogEnd;
	float m_FogDensity;
	
	// Alpha blending
	bool m_bAlphaBlendEnable;
	D3DBLEND m_SrcBlend;
	D3DBLEND m_DestBlend;
	
	// Depth/stencil
	bool m_bZEnable;
	bool m_bZWriteEnable;
	D3DCMPFUNC m_ZFunc;
	
	// Culling
	D3DCULL m_CullMode;
	
	// Flags
	bool m_bDirty;
};

//-----------------------------------------------------------------------------
// The main D3D8FF shader API implementation
//-----------------------------------------------------------------------------
class CShaderAPID3D8FF : public IShaderAPID3D8FF
{
public:
	CShaderAPID3D8FF();
	virtual ~CShaderAPID3D8FF();

	// Initialization
	bool OnDeviceInit();
	void OnDeviceShutdown();
	
	// PIX events
	virtual void BeginPIXEvent( unsigned long color, const char *szName );
	virtual void EndPIXEvent();
	virtual void AdvancePIXFrame();
	
	// Resource management
	virtual void ReleaseShaderObjects();
	virtual void RestoreShaderObjects();
	virtual void ResetRenderState( bool bFullReset = true );
	
	// Texture access
	virtual IDirect3DBaseTexture* GetD3DTexture( ShaderAPITextureHandle_t hTexture );
	
	// State management
	virtual void QueueResetRenderState();
	
	// Viewport
	virtual void SetViewports( int nCount, const ShaderViewport_t* pViewports );
	virtual int GetViewports( ShaderViewport_t* pViewports, int nMax ) const;
	
	// Buffer clearing
	virtual void ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil, int renderTargetWidth, int renderTargetHeight );
	virtual void ClearColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	
	// Shader binding (no-ops for fixed function)
	virtual void BindVertexShader( VertexShaderHandle_t hVertexShader ) {}
	virtual void BindGeometryShader( GeometryShaderHandle_t hGeometryShader ) {}
	virtual void BindPixelShader( PixelShaderHandle_t hPixelShader ) {}
	
	// Rasterizer state
	virtual void SetRasterState( const ShaderRasterState_t& state );
	
	// Mode setting (handled by device)
	virtual bool SetMode( void* hwnd, int nAdapter, const ShaderDeviceInfo_t &info ) { return false; }
	virtual void ChangeVideoMode( const ShaderDeviceInfo_t &info ) {}
	
	// Snapshot management
	virtual StateSnapshot_t TakeSnapshot();
	
	// Texture filtering
	virtual void TexMinFilter( ShaderTexFilterMode_t texFilterMode );
	virtual void TexMagFilter( ShaderTexFilterMode_t texFilterMode );
	virtual void TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode );
	
	// Copy render target
	virtual void CopyRenderTargetToTexture( ShaderAPITextureHandle_t textureHandle );
	
	// Material binding
	virtual void Bind( IMaterial* pMaterial );
	
	// Flush buffered primitives
	virtual void FlushBufferedPrimitives();
	
	// Dynamic mesh access
	virtual IMesh* GetDynamicMesh( IMaterial* pMaterial, int nHWSkinBoneCount, bool bBuffered = true,
		IMesh* pVertexOverride = 0, IMesh* pIndexOverride = 0);
	virtual IMesh* GetDynamicMeshEx( IMaterial* pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount, 
		bool bBuffered = true, IMesh* pVertexOverride = 0, IMesh* pIndexOverride = 0 );
	
	// Snapshot queries
	virtual bool IsTranslucent( StateSnapshot_t id ) const;
	virtual bool IsAlphaTested( StateSnapshot_t id ) const;
	virtual bool UsesVertexAndPixelShaders( StateSnapshot_t id ) const { return false; }
	virtual bool IsDepthWriteEnabled( StateSnapshot_t id ) const;
	
	// Vertex format
	virtual VertexFormat_t ComputeVertexFormat( int numSnapshots, StateSnapshot_t* pIds ) const;
	virtual VertexFormat_t ComputeVertexUsage( int numSnapshots, StateSnapshot_t* pIds ) const;
	
	// Rendering pass
	virtual void BeginPass( StateSnapshot_t snapshot );
	virtual void RenderPass( int nPass, int nPassCount );
	
	// Bone weights
	virtual void SetNumBoneWeights( int numBones );
	
	// Lighting
	virtual void SetLight( int lightNum, const LightDesc_t& desc );
	virtual void SetLightingOrigin( Vector vLightingOrigin );
	virtual void SetAmbientLight( float r, float g, float b );
	virtual void SetAmbientLightCube( Vector4D cube[6] );
	
	// Shade mode
	virtual void ShadeMode( ShaderShadeMode_t mode );
	
	// Cull mode
	virtual void CullMode( MaterialCullMode_t cullMode );
	
	// Depth functions
	virtual void ForceDepthFuncEquals( bool bEnable );
	virtual void OverrideDepthEnable( bool bEnable, bool bDepthEnable );
	
	// Height clip
	virtual void SetHeightClipZ( float z );
	virtual void SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode );
	
	// Clip planes
	virtual void SetClipPlane( int index, const float *pPlane );
	virtual void EnableClipPlane( int index, bool bEnable );
	
	// Skinning matrices
	virtual void SetSkinningMatrices();
	
	// Format queries
	virtual ImageFormat GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired = true ) const;
	virtual ImageFormat GetNearestRenderTargetFormat( ImageFormat fmt ) const;
	virtual bool DoRenderTargetsNeedSeparateDepthBuffer() const;
	
	// IShaderAPID3D8FF specific methods
	virtual void DrawMesh( CMeshBase *pMesh );
	virtual void DrawWithVertexAndIndexBuffers( void );
	virtual void ModifyVertexData( );
	virtual void GetBufferedState( BufferedState_t &state );
	virtual D3DCULL GetCullMode() const { return m_DynamicState.m_CullMode; }
	virtual void ComputeFillRate();
	virtual bool IsInSelectionMode() const { return false; }
	virtual void RegisterSelectionHit( float minz, float maxz ) {}
	virtual IMaterialInternal* GetBoundMaterial();
	virtual void ApplyZBias( const ShadowState_t &shaderState );
	virtual void ApplyTextureEnable( const ShadowState_t &state, int stage );
	virtual void ApplyCullEnable( bool bEnable );
	virtual void SetVertexBlendState( int numBones );
	virtual void ApplyFogMode( ShaderFogMode_t fogMode, bool bSRGBWritesEnabled, bool bDisableGammaCorrection );
	virtual int GetActualTextureStageCount() const { return D3D8FF_MAX_TEXTURE_STAGES; }
	virtual int GetActualSamplerCount() const { return D3D8FF_MAX_TEXTURE_STAGES; }
	virtual bool IsRenderingMesh() const { return m_bRenderingMesh; }
	virtual void FogMode( MaterialFogMode_t fogMode );
	virtual int GetCurrentFrameCounter( void ) const { return m_nFrameCounter; }
	virtual void SetupSelectionModeVisualizationState() {}
	virtual bool UsingSoftwareVertexProcessing() const;
	virtual void EnabledSRGBWrite( bool bEnabled ) {}
	virtual void ApplyAlphaToCoverage( bool bEnable ) {}
	virtual void PrintfVA( char *fmt, va_list vargs );
	virtual void Printf( PRINTF_FORMAT_STRING const char *fmt, ... );
	virtual float Knob( char *knobname, float *setvalue = NULL ) { return 0.0f; }
	
	// Advanced features (stubs)
	virtual void SetShadowDepthBiasFactors( float fShadowSlopeScaleDepthBias, float fShadowDepthBias );
	virtual void BindVertexBuffer( int nStreamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes, int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions = 1 );
	virtual void BindIndexBuffer( IIndexBuffer *pIndexBuffer, int nOffsetInBytes );
	virtual void Draw( MaterialPrimitiveType_t primitiveType, int nFirstIndex, int nIndexCount );
	virtual void PerformFullScreenStencilOperation( void );
	virtual void SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor );
	virtual bool SupportsCSAAMode( int nNumSamples, int nQualityLevel );
	virtual void InvalidateDelayedShaderConstants( void );
	virtual float GammaToLinear_HardwareSpecific( float fGamma ) const;
	virtual float LinearToGamma_HardwareSpecific( float fLinear ) const;
	virtual void SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture );
	virtual ImageFormat GetNullTextureFormat( void );
	virtual void BindVertexTexture( VertexTextureSampler_t nSampler, ShaderAPITextureHandle_t textureHandle );
	virtual void EnableHWMorphing( bool bEnable );
	virtual void SetFlexWeights( int nFirstWeight, int nCount, const MorphWeight_t* pWeights );
	virtual void FogMaxDensity( float flMaxDensity );
	virtual void CreateTextures( ShaderAPITextureHandle_t *pHandles, int count, int width, int height, int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags, const char *pDebugName, const char *pTextureGroupName );
	virtual void AcquireThreadOwnership();
	virtual void ReleaseThreadOwnership();
	virtual bool SupportsNormalMapCompression() const;
	virtual void EnableBuffer2FramesAhead( bool bEnable );
	virtual void SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale );
	virtual void OverrideAlphaWriteEnable( bool bEnable, bool bAlphaWriteEnable );
	virtual void OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable );
	virtual void ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth );
	virtual void CopyRenderTargetToScratchTexture( ShaderAPITextureHandle_t srcRt, ShaderAPITextureHandle_t dstTex, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL );
	virtual void LockRect( void** pOutBits, int* pOutPitch, ShaderAPITextureHandle_t texHandle, int mipmap, int x, int y, int w, int h, bool bWrite, bool bRead );
	virtual void UnlockRect( ShaderAPITextureHandle_t texHandle, int mipmap );
	virtual void TexLodClamp( int finest );
	virtual void TexLodBias( float bias );
	virtual void CopyTextureToTexture( ShaderAPITextureHandle_t srcTex, ShaderAPITextureHandle_t dstTex );
	virtual void EnableAlphaToCoverage();
	virtual void DisableAlphaToCoverage();
	virtual void ComputeVertexDescription( unsigned char* pBuffer, VertexFormat_t vertexFormat, MeshDesc_t& desc ) const;
	virtual bool SupportsShadowDepthTextures( void );
	virtual void SetDisallowAccess( bool );
	virtual void EnableShaderShaderMutex( bool );
	virtual void ShaderLock();
	virtual void ShaderUnlock();
	virtual ImageFormat GetShadowDepthTextureFormat( void );
	virtual bool SupportsFetch4( void );
	virtual IMesh *GetFlexMesh();
	virtual void SetFlashlightStateEx( const FlashlightState_t &state, const VMatrix &worldToTexture, ITexture *pFlashlightDepthTexture );
	virtual bool SupportsMSAAMode( int nMSAAMode );
	virtual bool OwnGPUResources( bool bEnable );
	virtual void GetFogDistances( float *fStart, float *fEnd, float *fFogZ );
	virtual void SetPIXMarker( unsigned long color, const char *szName );
	virtual void DisableAllLocalLights();
	virtual int CompareSnapshots( StateSnapshot_t snapshot0, StateSnapshot_t snapshot1 );
	virtual void GetMaxToRender( IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices );
	virtual int GetMaxVerticesToRender( IMaterial *pMaterial );
	virtual int GetMaxIndicesToRender();
	virtual void UserClipTransform( const VMatrix &worldToView );
	virtual MorphFormat_t ComputeMorphFormat( int numSnapshots, StateSnapshot_t* pIds ) const;
	virtual void SetRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t colorTextureHandle = SHADER_RENDERTARGET_BACKBUFFER, ShaderAPITextureHandle_t depthTextureHandle = SHADER_RENDERTARGET_DEPTHBUFFER );
	virtual void CopyRenderTargetToTextureEx( ShaderAPITextureHandle_t textureHandle, int nRenderTargetID, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL );
	virtual void CopyTextureToRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t textureHandle, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL );
	virtual void HandleDeviceLost();
	virtual void EnableLinearColorSpaceFrameBuffer( bool bEnable );
	virtual void SetFullScreenTextureHandle( ShaderAPITextureHandle_t h );
	virtual void SetFastClipPlane( const float *pPlane );
	virtual void EnableFastClip( bool bEnable );
	virtual void EnableUserClipTransformOverride( bool bEnable );
	virtual int OcclusionQuery_GetNumPixelsRendered( ShaderAPIOcclusionQuery_t hQuery, bool bFlush = false );
	virtual void SetFlashlightState( const FlashlightState_t &state, const VMatrix &worldToTexture );
	virtual void ClearVertexAndPixelShaderRefCounts();
	virtual void PurgeUnusedVertexAndPixelShaders();
	virtual void DXSupportLevelChanged();
	virtual void EvictManagedResources();
	virtual void SetAnisotropicLevel( int nAnisotropyLevel );
	virtual void SyncToken( const char *pToken );
	virtual void SetStandardVertexShaderConstants( float fOverbright );
	virtual ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject( void );
	virtual void DestroyOcclusionQueryObject( ShaderAPIOcclusionQuery_t );
	virtual void BeginOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t );
	virtual void EndOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t );
	virtual int SelectionMode( bool selectionMode );
	virtual void SelectionBuffer( unsigned int* pBuffer, int size );
	virtual void ClearSelectionNames();
	virtual void LoadSelectionName( int name );
	virtual void PushSelectionName( int name );
	virtual void PopSelectionName();
	virtual void ForceHardwareSync();
	virtual void ClearSnapshots();
	virtual void FogStart( float fStart );
	virtual void FogEnd( float fEnd );
	virtual void SetFogZ( float fogZ );
	virtual void SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void GetSceneFogColor( unsigned char *rgb );
	virtual void SceneFogMode( MaterialFogMode_t fogMode );
	virtual bool CanDownloadTextures() const;
	virtual int GetCurrentDynamicVBSize( void );
	virtual void DestroyVertexBuffers( bool bExitingLevel = false );
	virtual void ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat );
	virtual void ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride );
	virtual void FlushHardware();
	virtual void BeginFrame();
	virtual void EndFrame();
	virtual void SetRenderTarget( ShaderAPITextureHandle_t colorTextureHandle = SHADER_RENDERTARGET_BACKBUFFER, ShaderAPITextureHandle_t depthTextureHandle = SHADER_RENDERTARGET_DEPTHBUFFER );
	virtual void ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth );
	virtual void TexSetPriority( int priority );
	virtual void BindTexture( Sampler_t sampler, ShaderAPITextureHandle_t textureHandle );
	virtual void SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numBools = 1, bool bForce = false );
	virtual void SetIntegerVertexShaderConstant( int var, int const* pVec, int numIntVecs = 1, bool bForce = false );
	virtual void SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools = 1, bool bForce = false );
	virtual void SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs = 1, bool bForce = false );
	virtual bool ShouldWriteDepthToDestAlpha( void ) const;
	virtual void PushDeformation( DeformationBase_t const *Deformation );
	virtual void PopDeformation();
	virtual void MarkUnusedVertexFields( unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords );
	virtual void SetPSNearAndFarZ( int pshReg );
	virtual void DeleteTexture( ShaderAPITextureHandle_t textureHandle );
	virtual bool IsTexture( ShaderAPITextureHandle_t textureHandle );
	virtual bool IsTextureResident( ShaderAPITextureHandle_t textureHandle );
	virtual void ModifyTexture( ShaderAPITextureHandle_t textureHandle );
	virtual void TexImageFromVTF( IVTFTexture* pVTF, int iVTFFrame );
	virtual bool TexLock( int level, int cubeFaceID, int xOffset, int yOffset, int width, int height, CPixelWriter& writer );
	virtual void TexUnlock();
	
	// Missing IShaderAPI abstract methods
	virtual void ClearStencilBufferRectangle( int xmin, int ymin, int xmax, int ymax, int stencilValue );
	virtual void GetDXLevelDefaults( uint &max_dxlevel, uint &recommended_dxlevel );
	virtual const FlashlightState_t& GetFlashlightStateEx( VMatrix &worldToTexture, ITexture **pFlashlightDepthTexture ) const;
	virtual float GetAmbientLightCubeLuminance( void );
	virtual void GetDX9LightState( LightState_t *state ) const;
	virtual int GetPixelFogCombo( void );
	virtual void BindStandardVertexTexture( VertexTextureSampler_t sampler, StandardTextureId_t id );
	virtual bool IsHWMorphingEnabled( void ) const;
	virtual void GetStandardTextureDimensions( int *pWidth, int *pHeight, StandardTextureId_t id );
	virtual int GetNumActiveDeformations( void ) const;
	virtual int GetPackedDeformationInformation( int nMaskOfUnderstoodDeformations, float *pConstantValuesOut, int nBufferSize, int nMaximumDeformations, int *pNumDefsOut ) const;
	virtual void ExecuteCommandBuffer( uint8 *pCmdBuffer );
	virtual void SetStandardTextureHandle( StandardTextureId_t id, ShaderAPITextureHandle_t handle );
	virtual void GetCurrentColorCorrection( ShaderColorCorrectionInfo_t *pInfo );
	virtual ShaderAPITextureHandle_t CreateTexture( int width, int height, int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags, const char *pDebugName, const char *pTextureGroupName );
	virtual ShaderAPITextureHandle_t CreateDepthTexture( ImageFormat renderTargetFormat, int width, int height, const char *pDebugName, bool bTexture );
	virtual void TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset, int width, int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData );
	virtual void TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset, int zOffset, int width, int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData );
	
	// Stencil operations
	virtual void SetStencilEnable( bool bEnable );
	virtual void SetStencilFailOperation( StencilOperation_t op );
	virtual void SetStencilZFailOperation( StencilOperation_t op );
	virtual void SetStencilPassOperation( StencilOperation_t op );
	virtual void SetStencilCompareFunction( StencilComparisonFunction_t cmpfn );
	virtual void SetStencilReferenceValue( int ref );
	virtual void SetStencilTestMask( uint32 msk );
	virtual void SetStencilWriteMask( uint32 msk );
	
	// Rendering parameter accessors
	virtual float GetFloatRenderingParameter( int parm_number ) const;
	virtual int GetIntRenderingParameter( int parm_number ) const;
	virtual Vector GetVectorRenderingParameter( int parm_number ) const;
	virtual void SetFloatRenderingParameter( int parm_number, float value );
	virtual void SetIntRenderingParameter( int parm_number, int value );
	virtual void SetVectorRenderingParameter( int parm_number, const Vector &value );
	
	// Projection matrix setup
	virtual void PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right );
	
	// Additional IShaderDynamicAPI methods
	virtual void BindStandardTexture( Sampler_t sampler, StandardTextureId_t id );
	virtual ITexture *GetRenderTargetEx( int nRenderTargetID );
	virtual void SetToneMappingScaleLinear( const Vector &scale );
	virtual const Vector &GetToneMappingScaleLinear( void ) const;
	virtual float GetLightMapScaleFactor( void ) const;
	virtual void LoadBoneMatrix( int boneIndex, const float *pMatrix );
	virtual const LightDesc_t &GetLight( int lightNum ) const;
	virtual void SetPixelShaderFogParams( int reg );
	virtual void SetVertexShaderStateAmbientLightCube();
	virtual void SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack = false );
	virtual void CommitPixelShaderLighting( int pshReg );
	virtual CMeshBuilder *GetVertexModifyBuilder();
	virtual bool InFlashlightMode() const;
	virtual const FlashlightState_t &GetFlashlightState( VMatrix &worldToTexture ) const;
	virtual bool InEditorMode() const;
	virtual MorphFormat_t GetBoundMorphFormat();
	virtual void SetBumpEnvMatrix( TextureStage_t textureStage, float m00, float m01, float m10, float m11 );
	virtual void SetVertexShaderIndex( int vshIndex );
	virtual void SetPixelShaderIndex( int pshIndex );
	virtual void GetBackBufferDimensions( int &width, int &height ) const;
	virtual int GetMaxLights() const;
	virtual void Color4ubv( const unsigned char *rgba );
	virtual void SetVertexShaderConstant( int var, const float *pVec, int numConst = 1, bool bForce = false );
	virtual void SetPixelShaderConstant( int var, const float *pVec, int numConst = 1, bool bForce = false );
	virtual void SetDefaultState();
	virtual void GetWorldSpaceCameraPosition( float *pPos ) const;
	virtual int GetCurrentNumBones() const;
	virtual int GetCurrentLightCombo() const;
	virtual MaterialFogMode_t GetCurrentFogType() const;
	virtual void SetTextureTransformDimension( TextureStage_t textureStage, int dimension, bool projected );
	virtual void DisableTextureTransform( TextureStage_t textureStage );
	
	// Immediate mode transform/color (not typically used)
	virtual void Translate( float x, float y, float z );
	virtual void Scale( float x, float y, float z );
	virtual void ScaleXY( float x, float y );
	virtual void Color3f( float r, float g, float b );
	virtual void Color3fv( const float *rgb );
	virtual void Color4f( float r, float g, float b, float a );
	virtual void Color4fv( const float *rgba );
	virtual void Color3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void Color3ubv( const unsigned char *rgb );
	virtual void Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	
	// Immediate mode matrix operations
	virtual void LoadMatrix( float *m );
	virtual void MultMatrix( float *m );
	virtual void MultMatrixLocal( float *m );
	virtual void GetMatrix( MaterialMatrixMode_t matrixMode, float *dst );
	virtual void LoadIdentity();
	virtual void LoadCameraToWorld();
	virtual void Ortho( double left, double right, double bottom, double top, double zNear, double zFar );
	virtual void PerspectiveX( double fovx, double aspect, double zNear, double zFar );
	virtual void PickMatrix( int x, int y, int width, int height );
	virtual void Rotate( float angle, float x, float y, float z );
	virtual double CurrentTime() const;
	virtual void GetLightmapDimensions( int *w, int *h );
	virtual MaterialFogMode_t GetSceneFogMode();
	virtual void MatrixMode( MaterialMatrixMode_t matrixMode );
	virtual void PushMatrix();
	virtual void PopMatrix();
	
	// D3D8FF specific
	virtual void SetTextureStageState( int stage, const D3D8FFTextureStageState_t &state );
	virtual void SetFixedFunctionMaterial( const D3D8FFMaterialState_t &material );
	virtual void SetFixedFunctionLight( int lightNum, const D3DLIGHT9 &light );
	virtual void EnableFixedFunctionLight( int lightNum, bool bEnable );

private:
	// Apply render states to D3D device
	void CommitStateChanges();
	void ApplyTextureStageState( int stage );
	void ApplyMaterialState();
	void ApplyLightingState();
	void ApplyFogState();
	void ApplyRenderState();
	
	// State tracking
	struct DynamicState_t
	{
		D3DCULL m_CullMode;
		D3DVIEWPORT9 m_Viewport;
		D3DCOLOR m_ClearColor;
		bool m_bClearColorDirty;
	};
	
	DynamicState_t m_DynamicState;
	ShadowState_t m_CurrentShadowState;
	CUtlVector<ShadowState_t> m_SnapshotList;
	
	// Current state
	IMaterialInternal *m_pBoundMaterial;
	bool m_bRenderingMesh;
	int m_nFrameCounter;
	int m_nCurrentSnapshot;
	
	// Lights
	D3DLIGHT9 m_Lights[D3D8FF_MAX_LIGHTS];
	bool m_bLightEnabled[D3D8FF_MAX_LIGHTS];
	bool m_bLightsDirty;
};

// Singleton accessor
CShaderAPID3D8FF* ShaderAPI();

#endif // SHADERAPID3D8FF_IMPL_H

