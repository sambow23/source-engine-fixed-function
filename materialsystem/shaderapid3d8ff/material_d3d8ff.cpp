//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Material System Implementation - Shader Fallbacks
//
//===========================================================================//

#include "material_d3d8ff.h"
#include "shaderdeviced3d8ff.h"
#include "tier0/dbg.h"
#include "tier1/strtools.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Init/Shutdown
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::Init()
{
	Msg( "D3D8FF: Material fallback system initialized\n" );
}

void CMaterialFallbackD3D8FF::Shutdown()
{
	Msg( "D3D8FF: Material fallback system shutdown\n" );
}

//-----------------------------------------------------------------------------
// Main translation function
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateMaterial( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	if ( !pMaterial )
		return false;
	
	// Initialize state to defaults
	memset( &state, 0, sizeof(state) );
	state.m_hBaseTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
	state.m_hLightmap = INVALID_SHADERAPI_TEXTURE_HANDLE;
	state.m_hDetail = INVALID_SHADERAPI_TEXTURE_HANDLE;
	state.m_hEnvmap = INVALID_SHADERAPI_TEXTURE_HANDLE;
	state.m_Diffuse = D3DCOLOR_XRGB( 255, 255, 255 );
	state.m_Ambient = D3DCOLOR_XRGB( 255, 255, 255 );
	state.m_Specular = D3DCOLOR_XRGB( 0, 0, 0 );
	state.m_Emissive = D3DCOLOR_XRGB( 0, 0, 0 );
	state.m_flShininess = 0.0f;
	state.m_bLightingEnabled = true;
	state.m_nNumTextureStages = 1;
	
	// Get shader name
	const char *pShaderName = pMaterial->GetShaderName();
	if ( !pShaderName )
		return false;
	
	// Translate based on shader type
	bool bSuccess = false;
	
	if ( !V_stricmp( pShaderName, "LightmappedGeneric" ) || 
	     !V_stricmp( pShaderName, "WorldVertexTransition" ) ||
	     !V_stricmp( pShaderName, "Lightmapped_4WayBlend" ) )
	{
		bSuccess = TranslateLightmappedGeneric( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "VertexLitGeneric" ) )
	{
		bSuccess = TranslateVertexLitGeneric( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "UnlitGeneric" ) )
	{
		bSuccess = TranslateUnlitGeneric( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "UnlitTwoTexture" ) )
	{
		bSuccess = TranslateUnlitTwoTexture( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "Modulate" ) )
	{
		bSuccess = TranslateModulate( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "Sky" ) || !V_stricmp( pShaderName, "SkyBox" ) )
	{
		bSuccess = TranslateSky( pMaterial, state );
	}
	else if ( !V_stricmp( pShaderName, "Water" ) || !V_stricmp( pShaderName, "Refract" ) )
	{
		bSuccess = TranslateWater( pMaterial, state );
	}
	else
	{
		// Default fallback: simple textured
		Warning( "D3D8FF: Unknown shader '%s', using default fallback\n", pShaderName );
		bSuccess = TranslateUnlitGeneric( pMaterial, state );
	}
	
	if ( bSuccess )
	{
		// Translate material flags
		TranslateMaterialFlags( pMaterial, state );
		TranslateBlendMode( pMaterial, state );
	}
	
	return bSuccess;
}

//-----------------------------------------------------------------------------
// LightmappedGeneric translation
// Stage 0: Base texture
// Stage 1: Lightmap (modulate2x)
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateLightmappedGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Get lightmap
	IMaterialVar *pLightmap = pMaterial->FindVar( "$lightmap", NULL, false );
	if ( pLightmap && pLightmap->IsTexture() )
	{
		state.m_hLightmap = GetTextureHandle( pLightmap );
	}
	
	// Stage 0: Base texture
	SetupTextureStage( state, 0,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	
	// Stage 1: Lightmap (modulate 2x)
	if ( state.m_hLightmap != INVALID_SHADERAPI_TEXTURE_HANDLE )
	{
		SetupTextureStage( state, 1,
			D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_CURRENT,
			D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_CURRENT );
		state.m_nNumTextureStages = 2;
	}
	else
	{
		state.m_nNumTextureStages = 1;
	}
	
	// Lighting disabled (lightmap provides lighting)
	state.m_bLightingEnabled = false;
	
	return true;
}

//-----------------------------------------------------------------------------
// VertexLitGeneric translation
// Stage 0: Base texture * vertex color (lighting)
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateVertexLitGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Check for self-illumination
	IMaterialVar *pSelfIllum = pMaterial->FindVar( "$selfillum", NULL, false );
	bool bSelfIllum = pSelfIllum && pSelfIllum->GetIntValue() != 0;
	
	// Stage 0: Base texture modulated with lighting
	SetupModulateStage( state, 0 );
	state.m_nNumTextureStages = 1;
	
	// Enable vertex lighting
	state.m_bLightingEnabled = true;
	state.m_bVertexColor = false; // Use D3D lighting, not vertex colors
	
	// Self-illumination
	if ( bSelfIllum )
	{
		state.m_Emissive = D3DCOLOR_XRGB( 255, 255, 255 );
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// UnlitGeneric translation
// Stage 0: Base texture (no lighting)
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateUnlitGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Stage 0: Simple texture lookup
	SetupTextureStage( state, 0,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	state.m_nNumTextureStages = 1;
	
	// No lighting
	state.m_bLightingEnabled = false;
	
	// Check for vertex color
	IMaterialVar *pVertexColor = pMaterial->FindVar( "$vertexcolor", NULL, false );
	if ( pVertexColor && pVertexColor->GetIntValue() != 0 )
	{
		// Modulate with vertex color
		SetupTextureStage( state, 0,
			D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE,
			D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE );
		state.m_bVertexColor = true;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// UnlitTwoTexture translation
// Stage 0: Base texture
// Stage 1: Second texture (modulate)
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateUnlitTwoTexture( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Get second texture
	IMaterialVar *pTexture2 = pMaterial->FindVar( "$texture2", NULL, false );
	ShaderAPITextureHandle_t hTexture2 = INVALID_SHADERAPI_TEXTURE_HANDLE;
	if ( pTexture2 && pTexture2->IsTexture() )
	{
		hTexture2 = GetTextureHandle( pTexture2 );
	}
	
	// Stage 0: Base texture
	SetupTextureStage( state, 0,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	
	// Stage 1: Second texture (modulate)
	if ( hTexture2 != INVALID_SHADERAPI_TEXTURE_HANDLE )
	{
		SetupTextureStage( state, 1,
			D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT,
			D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT );
		state.m_nNumTextureStages = 2;
		state.m_hDetail = hTexture2; // Store in detail slot
	}
	else
	{
		state.m_nNumTextureStages = 1;
	}
	
	// No lighting
	state.m_bLightingEnabled = false;
	
	return true;
}

//-----------------------------------------------------------------------------
// WorldVertexTransition translation
// Simplified: just use base texture with lightmap
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateWorldVertexTransition( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Fallback to lightmapped generic
	return TranslateLightmappedGeneric( pMaterial, state );
}

//-----------------------------------------------------------------------------
// Modulate translation
// Stage 0: Texture * vertex color
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateModulate( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Stage 0: Texture * diffuse (vertex color or white)
	SetupTextureStage( state, 0,
		D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	state.m_nNumTextureStages = 1;
	
	// No lighting, use vertex color
	state.m_bLightingEnabled = false;
	state.m_bVertexColor = true;
	
	// Blend mode: modulate (dest * src)
	state.m_bAlphaBlend = true;
	state.m_SrcBlend = D3DBLEND_DESTCOLOR;
	state.m_DestBlend = D3DBLEND_ZERO;
	
	return true;
}

//-----------------------------------------------------------------------------
// Sky translation
// Stage 0: Sky texture (no lighting, no fog)
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateSky( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Simple texture lookup
	SetupTextureStage( state, 0,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	state.m_nNumTextureStages = 1;
	
	// No lighting
	state.m_bLightingEnabled = false;
	
	return true;
}

//-----------------------------------------------------------------------------
// Water translation (simplified)
// Stage 0: Base texture
//-----------------------------------------------------------------------------
bool CMaterialFallbackD3D8FF::TranslateWater( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get base texture
	IMaterialVar *pBaseTexture = pMaterial->FindVar( "$basetexture", NULL, false );
	if ( pBaseTexture && pBaseTexture->IsTexture() )
	{
		state.m_hBaseTexture = GetTextureHandle( pBaseTexture );
	}
	
	// Simple texture lookup
	SetupTextureStage( state, 0,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE );
	state.m_nNumTextureStages = 1;
	
	// No lighting, enable alpha blending for translucency
	state.m_bLightingEnabled = false;
	state.m_bAlphaBlend = true;
	state.m_SrcBlend = D3DBLEND_SRCALPHA;
	state.m_DestBlend = D3DBLEND_INVSRCALPHA;
	
	return true;
}

//-----------------------------------------------------------------------------
// Helper: Get texture handle from material var
//-----------------------------------------------------------------------------
ShaderAPITextureHandle_t CMaterialFallbackD3D8FF::GetTextureHandle( IMaterialVar *pVar )
{
	if ( !pVar || !pVar->IsTexture() )
		return INVALID_SHADERAPI_TEXTURE_HANDLE;
	
	ITexture *pTexture = pVar->GetTextureValue();
	if ( !pTexture )
		return INVALID_SHADERAPI_TEXTURE_HANDLE;
	
	// Get texture handle from texture interface
	// Note: This might need to be implemented differently depending on your texture system
	return (ShaderAPITextureHandle_t)pTexture;
}

//-----------------------------------------------------------------------------
// Helper: Get color from material var
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::GetMaterialColor( IMaterialVar *pVar, D3DCOLOR &color )
{
	if ( !pVar )
	{
		color = D3DCOLOR_XRGB( 255, 255, 255 );
		return;
	}
	
	float r, g, b;
	pVar->GetVecValue( &r, &g, &b );
	
	int ir = (int)(r * 255.0f);
	int ig = (int)(g * 255.0f);
	int ib = (int)(b * 255.0f);
	
	color = D3DCOLOR_XRGB( ir, ig, ib );
}

//-----------------------------------------------------------------------------
// Helper: Setup texture stage
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::SetupTextureStage( FixedFunctionMaterialState_t &state, int stage,
	D3DTEXTUREOP colorOp, DWORD colorArg1, DWORD colorArg2,
	D3DTEXTUREOP alphaOp, DWORD alphaArg1, DWORD alphaArg2 )
{
	if ( stage < 0 || stage >= 4 )
		return;
	
	state.m_ColorOp[stage] = colorOp;
	state.m_ColorArg1[stage] = colorArg1;
	state.m_ColorArg2[stage] = colorArg2;
	state.m_AlphaOp[stage] = alphaOp;
	state.m_AlphaArg1[stage] = alphaArg1;
	state.m_AlphaArg2[stage] = alphaArg2;
}

//-----------------------------------------------------------------------------
// Helper: Setup modulate stage (texture * diffuse)
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::SetupModulateStage( FixedFunctionMaterialState_t &state, int stage )
{
	SetupTextureStage( state, stage,
		D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE );
}

//-----------------------------------------------------------------------------
// Helper: Setup add stage
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::SetupAddStage( FixedFunctionMaterialState_t &state, int stage )
{
	SetupTextureStage( state, stage,
		D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_CURRENT,
		D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_CURRENT );
}

//-----------------------------------------------------------------------------
// Helper: Disable stage
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::DisableStage( FixedFunctionMaterialState_t &state, int stage )
{
	SetupTextureStage( state, stage,
		D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE,
		D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE );
}

//-----------------------------------------------------------------------------
// Translate material flags
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::TranslateMaterialFlags( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	// Get material flags
	int flags = pMaterial->GetMaterialVarFlags();
	
	// Two-sided
	if ( flags & MATERIAL_VAR_NOCULL )
	{
		state.m_bTwoSided = true;
	}
	
	// Alpha test
	if ( flags & MATERIAL_VAR_ALPHATEST )
	{
		state.m_bAlphaTest = true;
		
		// Get alpha test reference
		IMaterialVar *pAlphaTestRef = pMaterial->FindVar( "$alphatestreference", NULL, false );
		if ( pAlphaTestRef )
		{
			state.m_flAlphaTestRef = pAlphaTestRef->GetFloatValue() / 255.0f;
		}
		else
		{
			state.m_flAlphaTestRef = 0.5f;
		}
	}
	
	// Vertex color
	if ( flags & MATERIAL_VAR_VERTEXCOLOR )
	{
		state.m_bVertexColor = true;
	}
}

//-----------------------------------------------------------------------------
// Translate blend mode
//-----------------------------------------------------------------------------
void CMaterialFallbackD3D8FF::TranslateBlendMode( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state )
{
	int flags = pMaterial->GetMaterialVarFlags();
	
	// Additive blending
	if ( flags & MATERIAL_VAR_ADDITIVE )
	{
		state.m_bAlphaBlend = true;
		state.m_SrcBlend = D3DBLEND_ONE;
		state.m_DestBlend = D3DBLEND_ONE;
	}
	// Translucent
	else if ( flags & MATERIAL_VAR_TRANSLUCENT )
	{
		state.m_bAlphaBlend = true;
		state.m_SrcBlend = D3DBLEND_SRCALPHA;
		state.m_DestBlend = D3DBLEND_INVSRCALPHA;
	}
	// Opaque (default)
	else
	{
		state.m_bAlphaBlend = false;
		state.m_SrcBlend = D3DBLEND_ONE;
		state.m_DestBlend = D3DBLEND_ZERO;
	}
}

//-----------------------------------------------------------------------------
// Apply fixed function material state to D3D9 device
//-----------------------------------------------------------------------------
void ApplyFixedFunctionMaterial( const FixedFunctionMaterialState_t &state )
{
	IDirect3DDevice9 *pDevice = Dx9Device();
	if ( !pDevice )
		return;
	
	// Set textures
	if ( state.m_hBaseTexture != INVALID_SHADERAPI_TEXTURE_HANDLE )
	{
		// TODO: Bind texture to stage 0
		// pDevice->SetTexture( 0, GetD3DTexture( state.m_hBaseTexture ) );
	}
	
	if ( state.m_hLightmap != INVALID_SHADERAPI_TEXTURE_HANDLE )
	{
		// TODO: Bind lightmap to stage 1
		// pDevice->SetTexture( 1, GetD3DTexture( state.m_hLightmap ) );
	}
	
	// Set texture stage states
	for ( int i = 0; i < state.m_nNumTextureStages; ++i )
	{
		pDevice->SetTextureStageState( i, D3DTSS_COLOROP, state.m_ColorOp[i] );
		pDevice->SetTextureStageState( i, D3DTSS_COLORARG1, state.m_ColorArg1[i] );
		pDevice->SetTextureStageState( i, D3DTSS_COLORARG2, state.m_ColorArg2[i] );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, state.m_AlphaOp[i] );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG1, state.m_AlphaArg1[i] );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, state.m_AlphaArg2[i] );
	}
	
	// Disable remaining stages
	for ( int i = state.m_nNumTextureStages; i < 4; ++i )
	{
		pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_DISABLE );
		pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	}
	
	// Set material properties
	D3DMATERIAL9 mat;
	ZeroMemory( &mat, sizeof(mat) );
	mat.Diffuse.r = ((state.m_Diffuse >> 16) & 0xFF) / 255.0f;
	mat.Diffuse.g = ((state.m_Diffuse >> 8) & 0xFF) / 255.0f;
	mat.Diffuse.b = ((state.m_Diffuse) & 0xFF) / 255.0f;
	mat.Diffuse.a = ((state.m_Diffuse >> 24) & 0xFF) / 255.0f;
	
	mat.Ambient.r = ((state.m_Ambient >> 16) & 0xFF) / 255.0f;
	mat.Ambient.g = ((state.m_Ambient >> 8) & 0xFF) / 255.0f;
	mat.Ambient.b = ((state.m_Ambient) & 0xFF) / 255.0f;
	mat.Ambient.a = ((state.m_Ambient >> 24) & 0xFF) / 255.0f;
	
	mat.Specular.r = ((state.m_Specular >> 16) & 0xFF) / 255.0f;
	mat.Specular.g = ((state.m_Specular >> 8) & 0xFF) / 255.0f;
	mat.Specular.b = ((state.m_Specular) & 0xFF) / 255.0f;
	mat.Specular.a = ((state.m_Specular >> 24) & 0xFF) / 255.0f;
	
	mat.Emissive.r = ((state.m_Emissive >> 16) & 0xFF) / 255.0f;
	mat.Emissive.g = ((state.m_Emissive >> 8) & 0xFF) / 255.0f;
	mat.Emissive.b = ((state.m_Emissive) & 0xFF) / 255.0f;
	mat.Emissive.a = ((state.m_Emissive >> 24) & 0xFF) / 255.0f;
	
	mat.Power = state.m_flShininess;
	
	pDevice->SetMaterial( &mat );
	
	// Set lighting
	pDevice->SetRenderState( D3DRS_LIGHTING, state.m_bLightingEnabled );
	
	// Set alpha blending
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, state.m_bAlphaBlend );
	if ( state.m_bAlphaBlend )
	{
		pDevice->SetRenderState( D3DRS_SRCBLEND, state.m_SrcBlend );
		pDevice->SetRenderState( D3DRS_DESTBLEND, state.m_DestBlend );
	}
	
	// Set alpha testing
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, state.m_bAlphaTest );
	if ( state.m_bAlphaTest )
	{
		pDevice->SetRenderState( D3DRS_ALPHAREF, (DWORD)(state.m_flAlphaTestRef * 255.0f) );
		pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	}
	
	// Set culling
	pDevice->SetRenderState( D3DRS_CULLMODE, state.m_bTwoSided ? D3DCULL_NONE : D3DCULL_CCW );
}

