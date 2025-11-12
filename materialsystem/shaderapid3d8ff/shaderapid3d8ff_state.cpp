//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF State Management and Commit Functions
//
//===========================================================================//

#include "shaderapid3d8ff_impl.h"
#include "shaderdeviced3d8ff.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Commit all state changes to D3D device
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::CommitStateChanges()
{
	if ( !Dx9Device() )
		return;
	
	if ( !m_CurrentShadowState.m_bDirty )
		return;
	
	// Apply all state categories
	ApplyRenderState();
	ApplyTextureStageState( 0 ); // Apply for all stages in loop
	ApplyMaterialState();
	ApplyLightingState();
	ApplyFogState();
	
	m_CurrentShadowState.m_bDirty = false;
}

//-----------------------------------------------------------------------------
// Apply render states
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ApplyRenderState()
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Depth/stencil states
	pDevice->SetRenderState( D3DRS_ZENABLE, m_CurrentShadowState.m_bZEnable ? D3DZB_TRUE : D3DZB_FALSE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, m_CurrentShadowState.m_bZWriteEnable );
	pDevice->SetRenderState( D3DRS_ZFUNC, m_CurrentShadowState.m_ZFunc );
	
	// Alpha blending
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, m_CurrentShadowState.m_bAlphaBlendEnable );
	if ( m_CurrentShadowState.m_bAlphaBlendEnable )
	{
		pDevice->SetRenderState( D3DRS_SRCBLEND, m_CurrentShadowState.m_SrcBlend );
		pDevice->SetRenderState( D3DRS_DESTBLEND, m_CurrentShadowState.m_DestBlend );
	}
	
	// Culling
	pDevice->SetRenderState( D3DRS_CULLMODE, m_CurrentShadowState.m_CullMode );
}

//-----------------------------------------------------------------------------
// Apply texture stage states for all stages
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ApplyTextureStageState( int stage )
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Apply all texture stages
	for ( int i = 0; i < D3D8FF_MAX_TEXTURE_STAGES; ++i )
	{
		const D3D8FFTextureStageState_t &state = m_CurrentShadowState.m_TextureStage[i];
		
		// Set texture stage operations
		pDevice->SetTextureStageState( i, D3DTSS_COLOROP, state.m_ColorOp );
		pDevice->SetTextureStageState( i, D3DTSS_COLORARG1, state.m_ColorArg1 );
		pDevice->SetTextureStageState( i, D3DTSS_COLORARG2, state.m_ColorArg2 );
		
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, state.m_AlphaOp );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG1, state.m_AlphaArg1 );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, state.m_AlphaArg2 );
		
		// Texture coordinate index
		pDevice->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, state.m_TexCoordIndex );
		
		// Texture transform
		pDevice->SetTextureStageState( i, D3DTSS_TEXTURETRANSFORMFLAGS, state.m_TexTransformFlags );
		
		// If this stage is disabled, we can stop here
		if ( state.m_ColorOp == D3DTOP_DISABLE )
			break;
	}
}

//-----------------------------------------------------------------------------
// Apply material properties
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ApplyMaterialState()
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Set material properties
	D3DMATERIAL9 material;
	memset( &material, 0, sizeof(material) );
	
	material.Diffuse = m_CurrentShadowState.m_Material.m_Diffuse;
	material.Ambient = m_CurrentShadowState.m_Material.m_Ambient;
	material.Specular = m_CurrentShadowState.m_Material.m_Specular;
	material.Emissive = m_CurrentShadowState.m_Material.m_Emissive;
	material.Power = m_CurrentShadowState.m_Material.m_Power;
	
	pDevice->SetMaterial( &material );
}

//-----------------------------------------------------------------------------
// Apply lighting state
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ApplyLightingState()
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Enable/disable lighting
	pDevice->SetRenderState( D3DRS_LIGHTING, m_CurrentShadowState.m_bLightingEnabled );
	
	// Set all lights if dirty
	if ( m_bLightsDirty )
	{
		for ( int i = 0; i < D3D8FF_MAX_LIGHTS; ++i )
		{
			if ( m_bLightEnabled[i] )
			{
				pDevice->SetLight( i, &m_Lights[i] );
				pDevice->LightEnable( i, TRUE );
			}
			else
			{
				pDevice->LightEnable( i, FALSE );
			}
		}
		m_bLightsDirty = false;
	}
}

//-----------------------------------------------------------------------------
// Apply fog state
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ApplyFogState()
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Enable/disable fog
	pDevice->SetRenderState( D3DRS_FOGENABLE, m_CurrentShadowState.m_bFogEnabled );
	
	if ( m_CurrentShadowState.m_bFogEnabled )
	{
		pDevice->SetRenderState( D3DRS_FOGCOLOR, m_CurrentShadowState.m_FogColor );
		pDevice->SetRenderState( D3DRS_FOGTABLEMODE, m_CurrentShadowState.m_FogMode );
		
		// Set fog parameters
		pDevice->SetRenderState( D3DRS_FOGSTART, *(DWORD*)(&m_CurrentShadowState.m_FogStart) );
		pDevice->SetRenderState( D3DRS_FOGEND, *(DWORD*)(&m_CurrentShadowState.m_FogEnd) );
		pDevice->SetRenderState( D3DRS_FOGDENSITY, *(DWORD*)(&m_CurrentShadowState.m_FogDensity) );
	}
}

//-----------------------------------------------------------------------------
// Set texture stage state
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetTextureStageState( int stage, const D3D8FFTextureStageState_t &state )
{
	if ( stage < 0 || stage >= D3D8FF_MAX_TEXTURE_STAGES )
		return;
	
	m_CurrentShadowState.m_TextureStage[stage] = state;
	m_CurrentShadowState.m_bDirty = true;
}

//-----------------------------------------------------------------------------
// Set fixed function material
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetFixedFunctionMaterial( const D3D8FFMaterialState_t &material )
{
	m_CurrentShadowState.m_Material = material;
	m_CurrentShadowState.m_bDirty = true;
}

//-----------------------------------------------------------------------------
// Set fixed function light
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetFixedFunctionLight( int lightNum, const D3DLIGHT9 &light )
{
	if ( lightNum < 0 || lightNum >= D3D8FF_MAX_LIGHTS )
		return;
	
	m_Lights[lightNum] = light;
	m_bLightsDirty = true;
}

//-----------------------------------------------------------------------------
// Enable/disable fixed function light
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::EnableFixedFunctionLight( int lightNum, bool bEnable )
{
	if ( lightNum < 0 || lightNum >= D3D8FF_MAX_LIGHTS )
		return;
	
	m_bLightEnabled[lightNum] = bEnable;
	m_bLightsDirty = true;
}

//-----------------------------------------------------------------------------
// Lighting methods
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetLight( int lightNum, const LightDesc_t& desc )
{
	if ( lightNum < 0 || lightNum >= D3D8FF_MAX_LIGHTS )
		return;
	
	D3DLIGHT9 light;
	memset( &light, 0, sizeof(light) );
	
	// Convert LightDesc_t to D3DLIGHT9
	switch ( desc.m_Type )
	{
	case MATERIAL_LIGHT_POINT:
		light.Type = D3DLIGHT_POINT;
		break;
	case MATERIAL_LIGHT_DIRECTIONAL:
		light.Type = D3DLIGHT_DIRECTIONAL;
		break;
	case MATERIAL_LIGHT_SPOT:
		light.Type = D3DLIGHT_SPOT;
		break;
	}
	
	// Position
	light.Position.x = desc.m_Position.x;
	light.Position.y = desc.m_Position.y;
	light.Position.z = desc.m_Position.z;
	
	// Direction
	light.Direction.x = desc.m_Direction.x;
	light.Direction.y = desc.m_Direction.y;
	light.Direction.z = desc.m_Direction.z;
	
	// Color
	light.Diffuse.r = desc.m_Color.x;
	light.Diffuse.g = desc.m_Color.y;
	light.Diffuse.b = desc.m_Color.z;
	light.Diffuse.a = 1.0f;
	
	// Attenuation
	light.Range = desc.m_Range;
	light.Attenuation0 = desc.m_Attenuation0;
	light.Attenuation1 = desc.m_Attenuation1;
	light.Attenuation2 = desc.m_Attenuation2;
	
	// Spot light params
	light.Theta = desc.m_Theta;
	light.Phi = desc.m_Phi;
	light.Falloff = desc.m_Falloff;
	
	SetFixedFunctionLight( lightNum, light );
	EnableFixedFunctionLight( lightNum, true );
}

void CShaderAPID3D8FF::SetLightingOrigin( Vector vLightingOrigin )
{
	// Store for future use
}

void CShaderAPID3D8FF::SetAmbientLight( float r, float g, float b )
{
	if ( !Dx9Device() )
		return;
	
	D3DCOLOR ambient = D3DCOLOR_COLORVALUE( r, g, b, 1.0f );
	Dx9Device()->SetRenderState( D3DRS_AMBIENT, ambient );
}

void CShaderAPID3D8FF::SetAmbientLightCube( Vector4D cube[6] )
{
	// Ambient cube not supported in simple fixed function
	// Just use average as ambient light
	float r = (cube[0].x + cube[1].x) * 0.5f;
	float g = (cube[0].y + cube[1].y) * 0.5f;
	float b = (cube[0].z + cube[1].z) * 0.5f;
	SetAmbientLight( r, g, b );
}

//-----------------------------------------------------------------------------
// Shade mode
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ShadeMode( ShaderShadeMode_t mode )
{
	if ( !Dx9Device() )
		return;
	
	D3DSHADEMODE shadeMode = D3DSHADE_GOURAUD;
	switch ( mode )
	{
	case SHADER_FLAT:
		shadeMode = D3DSHADE_FLAT;
		break;
	case SHADER_SMOOTH:
		shadeMode = D3DSHADE_GOURAUD;
		break;
	}
	
	Dx9Device()->SetRenderState( D3DRS_SHADEMODE, shadeMode );
}

//-----------------------------------------------------------------------------
// Cull mode
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::CullMode( MaterialCullMode_t cullMode )
{
	D3DCULL d3dCullMode = D3DCULL_CCW;
	
	switch ( cullMode )
	{
	case MATERIAL_CULLMODE_CCW:
		d3dCullMode = D3DCULL_CCW;
		break;
	case MATERIAL_CULLMODE_CW:
		d3dCullMode = D3DCULL_CW;
		break;
	}
	
	m_CurrentShadowState.m_CullMode = d3dCullMode;
	m_DynamicState.m_CullMode = d3dCullMode;
	m_CurrentShadowState.m_bDirty = true;
}

//-----------------------------------------------------------------------------
// Depth functions
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::ForceDepthFuncEquals( bool bEnable )
{
	if ( bEnable )
	{
		m_CurrentShadowState.m_ZFunc = D3DCMP_EQUAL;
	}
	else
	{
		m_CurrentShadowState.m_ZFunc = D3DCMP_LESSEQUAL;
	}
	m_CurrentShadowState.m_bDirty = true;
}

void CShaderAPID3D8FF::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
	if ( bEnable )
	{
		m_CurrentShadowState.m_bZEnable = bDepthEnable;
		m_CurrentShadowState.m_bDirty = true;
	}
}

//-----------------------------------------------------------------------------
// Height clip (not commonly used)
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetHeightClipZ( float z )
{
	// Not implemented for fixed function
}

void CShaderAPID3D8FF::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
	// Not implemented for fixed function
}

//-----------------------------------------------------------------------------
// Clip planes
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetClipPlane( int index, const float *pPlane )
{
	if ( !Dx9Device() || !pPlane )
		return;
	
	float plane[4] = { pPlane[0], pPlane[1], pPlane[2], pPlane[3] };
	Dx9Device()->SetClipPlane( index, plane );
}

void CShaderAPID3D8FF::EnableClipPlane( int index, bool bEnable )
{
	if ( !Dx9Device() )
		return;
	
	DWORD clipPlanes = 0;
	Dx9Device()->GetRenderState( D3DRS_CLIPPLANEENABLE, &clipPlanes );
	
	if ( bEnable )
	{
		clipPlanes |= (1 << index);
	}
	else
	{
		clipPlanes &= ~(1 << index);
	}
	
	Dx9Device()->SetRenderState( D3DRS_CLIPPLANEENABLE, clipPlanes );
}

//-----------------------------------------------------------------------------
// Skinning matrices
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::SetSkinningMatrices()
{
	// Fixed function skinning not implemented yet
}

//-----------------------------------------------------------------------------
// Format queries
//-----------------------------------------------------------------------------
ImageFormat CShaderAPID3D8FF::GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired ) const
{
	// TODO: Implement proper format conversion
	// For now, just return the requested format
	return fmt;
}

ImageFormat CShaderAPID3D8FF::GetNearestRenderTargetFormat( ImageFormat fmt ) const
{
	// TODO: Implement proper format conversion
	// For now, just return the requested format
	return fmt;
}

bool CShaderAPID3D8FF::DoRenderTargetsNeedSeparateDepthBuffer() const
{
	return false;
}

//-----------------------------------------------------------------------------
// Fog mode
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::FogMode( MaterialFogMode_t fogMode )
{
	m_CurrentShadowState.m_bFogEnabled = ( fogMode != MATERIAL_FOG_NONE );
	
	switch ( fogMode )
	{
	case MATERIAL_FOG_LINEAR:
		m_CurrentShadowState.m_FogMode = D3DFOG_LINEAR;
		break;
	case MATERIAL_FOG_LINEAR_BELOW_FOG_Z:
		m_CurrentShadowState.m_FogMode = D3DFOG_LINEAR;
		break;
	default:
		m_CurrentShadowState.m_FogMode = D3DFOG_NONE;
		break;
	}
	
	m_CurrentShadowState.m_bDirty = true;
}

//-----------------------------------------------------------------------------
// IShaderAPID3D8FF specific stubs
//-----------------------------------------------------------------------------
void CShaderAPID3D8FF::DrawMesh( CMeshBase *pMesh )
{
	// To be implemented when mesh system is ready
}

void CShaderAPID3D8FF::DrawWithVertexAndIndexBuffers( void )
{
	// To be implemented when mesh system is ready
}

void CShaderAPID3D8FF::ModifyVertexData( )
{
	// Not needed for fixed function
}

void CShaderAPID3D8FF::GetBufferedState( BufferedState_t &state )
{
	// Copy current state
	memcpy( &state.m_Viewport, &m_DynamicState.m_Viewport, sizeof(D3DVIEWPORT9) );
	// Textures and transforms to be filled in when texture system is ready
}

IMaterialInternal* CShaderAPID3D8FF::GetBoundMaterial()
{
	return m_pBoundMaterial;
}

void CShaderAPID3D8FF::ApplyZBias( const ShadowState_t &shaderState )
{
	// Z bias for decals, etc
	// To be implemented
}

void CShaderAPID3D8FF::ApplyTextureEnable( const ShadowState_t &state, int stage )
{
	// Texture enable handled in texture stage state
}

void CShaderAPID3D8FF::ApplyCullEnable( bool bEnable )
{
	if ( bEnable )
	{
		Dx9Device()->SetRenderState( D3DRS_CULLMODE, m_DynamicState.m_CullMode );
	}
	else
	{
		Dx9Device()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}
}

void CShaderAPID3D8FF::SetVertexBlendState( int numBones )
{
	// Vertex blending for skinning
	// To be implemented
}

void CShaderAPID3D8FF::ApplyFogMode( ShaderFogMode_t fogMode, bool bSRGBWritesEnabled, bool bDisableGammaCorrection )
{
	// Apply fog based on mode
	MaterialFogMode_t matFogMode = MATERIAL_FOG_NONE;
	
	switch ( fogMode )
	{
	case SHADER_FOGMODE_DISABLED:
		matFogMode = MATERIAL_FOG_NONE;
		break;
	case SHADER_FOGMODE_OO_OVERBRIGHT:
	case SHADER_FOGMODE_BLACK:
	case SHADER_FOGMODE_GREY:
	case SHADER_FOGMODE_FOGCOLOR:
	case SHADER_FOGMODE_WHITE:
		matFogMode = MATERIAL_FOG_LINEAR;
		break;
	}
	
	FogMode( matFogMode );
}

void CShaderAPID3D8FF::ComputeFillRate()
{
	// Debug/profiling function - not critical
}

