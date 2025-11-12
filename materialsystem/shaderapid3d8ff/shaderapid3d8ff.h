//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8 Fixed Function Shader API
//
//===========================================================================//

#ifndef SHADERAPID3D8FF_H
#define SHADERAPID3D8FF_H

#ifdef _WIN32
#pragma once
#endif

// Include Direct3D headers first
#include "locald3dtypes.h"

// Include shader API interfaces
#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "shaderapi/ishaderdynamic.h"

// Include local D3D8FF headers
#include "shaderapid3d8ff_global.h"
#include "shaderdeviced3d8ff.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CMeshBase;
class CMeshBuilder;
struct ShadowState_t;
class IMaterialInternal;

//-----------------------------------------------------------------------------
// State that matters to buffered meshes
//-----------------------------------------------------------------------------
struct BufferedState_t
{
	D3DXMATRIX m_Transform[3];
	D3DVIEWPORT9 m_Viewport;
	int m_BoundTexture[D3D8FF_MAX_TEXTURE_STAGES];
};

//-----------------------------------------------------------------------------
// Fixed function texture stage state
//-----------------------------------------------------------------------------
struct D3D8FFTextureStageState_t
{
	D3DTEXTUREOP m_ColorOp;
	DWORD m_ColorArg1;
	DWORD m_ColorArg2;
	D3DTEXTUREOP m_AlphaOp;
	DWORD m_AlphaArg1;
	DWORD m_AlphaArg2;
	DWORD m_TexCoordIndex;
	D3DTEXTURETRANSFORMFLAGS m_TexTransformFlags;
	bool m_bEnabled;
};

//-----------------------------------------------------------------------------
// Fixed function material state
//-----------------------------------------------------------------------------
struct D3D8FFMaterialState_t
{
	D3DCOLORVALUE m_Diffuse;
	D3DCOLORVALUE m_Ambient;
	D3DCOLORVALUE m_Specular;
	D3DCOLORVALUE m_Emissive;
	float m_Power;
};

//-----------------------------------------------------------------------------
// The D3D8FF shader API interface
//-----------------------------------------------------------------------------
class IShaderAPID3D8FF : public IShaderAPI
{
public:
	// Draws the mesh
	virtual void DrawMesh( CMeshBase *pMesh ) = 0;

	// Draw with currently bound vertex and index buffers
	virtual void DrawWithVertexAndIndexBuffers( void ) = 0;

	// Modifies vertex data when necessary
	virtual void ModifyVertexData( ) = 0;

	// Gets the current buffered state
	virtual void GetBufferedState( BufferedState_t &state ) = 0;

	// Gets the current backface cull state
	virtual D3DCULL GetCullMode() const = 0;

	// Measures fill rate
	virtual void ComputeFillRate() = 0;

	// Selection mode methods
	virtual bool IsInSelectionMode() const = 0;

	// We hit something in selection mode
	virtual void RegisterSelectionHit( float minz, float maxz ) = 0;

	// Get the currently bound material
	virtual IMaterialInternal* GetBoundMaterial() = 0;

	// These methods are called by the transition table
	virtual void ApplyZBias( const ShadowState_t &shaderState ) = 0;
	virtual void ApplyTextureEnable( const ShadowState_t &state, int stage ) = 0;
	virtual void ApplyCullEnable( bool bEnable ) = 0;
	virtual void SetVertexBlendState( int numBones ) = 0;
	virtual void ApplyFogMode( ShaderFogMode_t fogMode, bool bSRGBWritesEnabled, bool bDisableGammaCorrection ) = 0;

	virtual int GetActualTextureStageCount() const = 0;
	virtual int GetActualSamplerCount() const = 0;

	virtual bool IsRenderingMesh() const = 0;

	// Fog methods
	virtual void FogMode( MaterialFogMode_t fogMode ) = 0;

	virtual int GetCurrentFrameCounter( void ) const = 0;

	// Workaround hack for visualization of selection mode
	virtual void SetupSelectionModeVisualizationState() = 0;

	virtual bool UsingSoftwareVertexProcessing() const = 0;

	// Notification that SRGB write state is being changed
	virtual void EnabledSRGBWrite( bool bEnabled ) = 0;

	// Alpha to coverage
	virtual void ApplyAlphaToCoverage( bool bEnable ) = 0;

	virtual void PrintfVA( char *fmt, va_list vargs ) = 0;
	virtual void Printf( PRINTF_FORMAT_STRING const char *fmt, ... ) = 0;	
	virtual float Knob( char *knobname, float *setvalue = NULL ) = 0;

	// D3D8FF specific methods
	virtual void SetTextureStageState( int stage, const D3D8FFTextureStageState_t &state ) = 0;
	virtual void SetFixedFunctionMaterial( const D3D8FFMaterialState_t &material ) = 0;
	virtual void SetFixedFunctionLight( int lightNum, const D3DLIGHT9 &light ) = 0;
	virtual void EnableFixedFunctionLight( int lightNum, bool bEnable ) = 0;
};

#endif // SHADERAPID3D8FF_H

