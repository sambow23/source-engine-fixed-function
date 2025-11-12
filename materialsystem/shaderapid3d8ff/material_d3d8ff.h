//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Material System - Shader Fallbacks
//
//===========================================================================//

#ifndef MATERIAL_D3D8FF_H
#define MATERIAL_D3D8FF_H

#ifdef _WIN32
#pragma once
#endif

#include "locald3dtypes.h"
#include "materialsystem/imaterial.h"
#include "imaterialinternal.h"
#include "shaderapi/ishaderapi.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct IMaterialVar;

//-----------------------------------------------------------------------------
// Material parameter indices (from shader params)
//-----------------------------------------------------------------------------
enum MaterialParamIndex_t
{
	PARAM_BASETEXTURE = 0,
	PARAM_BASETEXTURE2,
	PARAM_LIGHTMAP,
	PARAM_DETAIL,
	PARAM_ENVMAP,
	PARAM_ENVMAPMASK,
	PARAM_BUMPMAP,
	PARAM_NORMALMAP,
	PARAM_SELFILLUM,
	PARAM_SELFILLUMMASK,
	PARAM_ALPHA,
	PARAM_COLOR,
	PARAM_COLOR2,
	PARAM_VERTEXCOLOR,
	PARAM_VERTEXALPHA,
	PARAM_TRANSLUCENT,
	PARAM_ALPHATEST,
	PARAM_ALPHATESTREFERENCE,
	PARAM_NOCULL,
	PARAM_ADDITIVE,
	PARAM_COUNT
};

//-----------------------------------------------------------------------------
// Fixed function material state
//-----------------------------------------------------------------------------
struct FixedFunctionMaterialState_t
{
	// Textures
	ShaderAPITextureHandle_t m_hBaseTexture;
	ShaderAPITextureHandle_t m_hLightmap;
	ShaderAPITextureHandle_t m_hDetail;
	ShaderAPITextureHandle_t m_hEnvmap;
	
	// Texture stage setup
	int m_nNumTextureStages;
	D3DTEXTUREOP m_ColorOp[4];
	DWORD m_ColorArg1[4];
	DWORD m_ColorArg2[4];
	D3DTEXTUREOP m_AlphaOp[4];
	DWORD m_AlphaArg1[4];
	DWORD m_AlphaArg2[4];
	
	// Material properties
	D3DCOLOR m_Diffuse;
	D3DCOLOR m_Ambient;
	D3DCOLOR m_Specular;
	D3DCOLOR m_Emissive;
	float m_flShininess;
	
	// Flags
	bool m_bLightingEnabled;
	bool m_bVertexColor;
	bool m_bAlphaBlend;
	bool m_bAlphaTest;
	float m_flAlphaTestRef;
	bool m_bTwoSided;
	
	// Blend modes
	D3DBLEND m_SrcBlend;
	D3DBLEND m_DestBlend;
};

//-----------------------------------------------------------------------------
// Material fallback system
//-----------------------------------------------------------------------------
class CMaterialFallbackD3D8FF
{
public:
	// Initialize/shutdown
	static void Init();
	static void Shutdown();
	
	// Translate a material to fixed function state
	static bool TranslateMaterial( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	
private:
	// Shader-specific translations
	static bool TranslateLightmappedGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateVertexLitGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateUnlitGeneric( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateUnlitTwoTexture( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateWorldVertexTransition( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateModulate( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateSky( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static bool TranslateWater( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	
	// Helper functions
	static ShaderAPITextureHandle_t GetTextureHandle( IMaterialVar *pVar );
	static void GetMaterialColor( IMaterialVar *pVar, D3DCOLOR &color );
	static void SetupTextureStage( FixedFunctionMaterialState_t &state, int stage,
		D3DTEXTUREOP colorOp, DWORD colorArg1, DWORD colorArg2,
		D3DTEXTUREOP alphaOp, DWORD alphaArg1, DWORD alphaArg2 );
	static void SetupModulateStage( FixedFunctionMaterialState_t &state, int stage );
	static void SetupAddStage( FixedFunctionMaterialState_t &state, int stage );
	static void DisableStage( FixedFunctionMaterialState_t &state, int stage );
	
	// Material flag translation
	static void TranslateMaterialFlags( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
	static void TranslateBlendMode( IMaterialInternal *pMaterial, FixedFunctionMaterialState_t &state );
};

//-----------------------------------------------------------------------------
// Apply fixed function material state to D3D9 device
//-----------------------------------------------------------------------------
void ApplyFixedFunctionMaterial( const FixedFunctionMaterialState_t &state );

#endif // MATERIAL_D3D8FF_H

