//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Mesh System Implementation
//
//===========================================================================//

#include "mesh_d3d8ff.h"
#include "shaderapid3d8ff_impl.h"
#include "shaderdeviced3d8ff.h"
#include "tier0/dbg.h"
#include "tier0/vprof.h"
#include "tier1/strtools.h"
#include "mathlib/vector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Global mesh manager
//-----------------------------------------------------------------------------
CMeshMgrD3D8FF g_MeshMgr;

//-----------------------------------------------------------------------------
// Vertex size calculation - simplified version
//-----------------------------------------------------------------------------
static int GetVertexSize( VertexFormat_t vertexFormat )
{
	int size = 0;
	
	// Position
	if ( vertexFormat & VERTEX_POSITION )
		size += 12; // 3 floats
	
	// Normal
	if ( vertexFormat & VERTEX_NORMAL )
		size += 12; // 3 floats
	
	// Color
	if ( vertexFormat & VERTEX_COLOR )
		size += 4; // D3DCOLOR
	
	// Texture coordinates - simplified, assume 2D texcoords
	// Check each texcoord slot
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		if ( vertexFormat & VERTEX_TEXCOORD_MASK(i) )
			size += 8; // 2 floats
	}
	
	return size > 0 ? size : 32; // Default to 32 bytes if nothing set
}

//-----------------------------------------------------------------------------
// Vertex Buffer Implementation
//-----------------------------------------------------------------------------
CVertexBufferD3D8FF::CVertexBufferD3D8FF( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup )
{
	m_pVertexBuffer = NULL;
	m_Type = type;
	m_VertexFormat = fmt;
	m_dwFVF = ComputeFVF( fmt );
	m_nVertexCount = nVertexCount;
	
	// TODO: Properly compute vertex size from format
	m_nVertexSize = 32; // Default size, should calculate based on format
	m_nBufferSize = m_nVertexCount * m_nVertexSize;
	m_bLocked = false;
	m_pLockedData = NULL;
	
	Create();
}

CVertexBufferD3D8FF::~CVertexBufferD3D8FF()
{
	Destroy();
}

bool CVertexBufferD3D8FF::Create()
{
	if ( !Dx9Device() )
		return false;
	
	DWORD usage = D3DUSAGE_WRITEONLY;
	D3DPOOL pool = D3DPOOL_MANAGED;
	
	if ( IsDynamic() )
	{
		usage |= D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	}
	
	HRESULT hr = Dx9Device()->CreateVertexBuffer(
		m_nBufferSize,
		usage,
		m_dwFVF,
		pool,
		&m_pVertexBuffer,
		NULL
	);
	
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to create vertex buffer (size=%d, FVF=0x%X)\n", m_nBufferSize, m_dwFVF );
		return false;
	}
	
	return true;
}

void CVertexBufferD3D8FF::Destroy()
{
	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = NULL;
	}
}

DWORD CVertexBufferD3D8FF::ComputeFVF( VertexFormat_t format )
{
	DWORD fvf = 0;
	
	// Position
	if ( format & VERTEX_POSITION )
		fvf |= D3DFVF_XYZ;
	
	// Normal
	if ( format & VERTEX_NORMAL )
		fvf |= D3DFVF_NORMAL;
	
	// Color
	if ( format & VERTEX_COLOR )
		fvf |= D3DFVF_DIFFUSE;
	
	// Texture coordinates
	int numTexCoords = 0;
	for ( int i = 0; i < 8; ++i )
	{
		if ( TexCoordSize( i, format ) > 0 )
			numTexCoords = i + 1;
	}
	
	fvf |= (numTexCoords << D3DFVF_TEXCOUNT_SHIFT);
	
	return fvf;
}

bool CVertexBufferD3D8FF::Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc )
{
	if ( !m_pVertexBuffer )
		return false;
	
	DWORD flags = 0;
	if ( bAppend )
		flags |= D3DLOCK_NOOVERWRITE;
	else if ( IsDynamic() )
		flags |= D3DLOCK_DISCARD;
	
	HRESULT hr = m_pVertexBuffer->Lock( 0, 0, &m_pLockedData, flags );
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to lock vertex buffer\n" );
		return false;
	}
	
	m_bLocked = true;
	
	// Fill in vertex desc
	desc.m_pPosition = (float*)m_pLockedData;
	desc.m_VertexSize_Position = m_nVertexSize;
	
	// Calculate offsets for other components
	int offset = 0;
	if ( m_VertexFormat & VERTEX_POSITION )
		offset += 12;
	
	if ( m_VertexFormat & VERTEX_NORMAL )
	{
		desc.m_pNormal = (float*)((unsigned char*)m_pLockedData + offset);
		desc.m_VertexSize_Normal = m_nVertexSize;
		offset += 12;
	}
	
	if ( m_VertexFormat & VERTEX_COLOR )
	{
		desc.m_pColor = (unsigned char*)m_pLockedData + offset;
		desc.m_VertexSize_Color = m_nVertexSize;
		offset += 4;
	}
	
	// Texture coordinates
	for ( int i = 0; i < 8; ++i )
	{
		int coordSize = TexCoordSize( i, m_VertexFormat );
		if ( coordSize > 0 )
		{
			desc.m_pTexCoord[i] = (float*)((unsigned char*)m_pLockedData + offset);
			desc.m_VertexSize_TexCoord[i] = m_nVertexSize;
			offset += coordSize * 4;
		}
	}
	
	return true;
}

void CVertexBufferD3D8FF::Unlock( int nVertexCount, VertexDesc_t &desc )
{
	if ( !m_bLocked )
		return;
	
	m_pVertexBuffer->Unlock();
	m_bLocked = false;
	m_pLockedData = NULL;
}

//-----------------------------------------------------------------------------
// Index Buffer Implementation
//-----------------------------------------------------------------------------
CIndexBufferD3D8FF::CIndexBufferD3D8FF( ShaderBufferType_t type, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup )
{
	m_pIndexBuffer = NULL;
	m_Type = type;
	m_IndexFormat = fmt;
	m_d3dFormat = ( fmt == MATERIAL_INDEX_FORMAT_16BIT ) ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
	m_nIndexCount = nIndexCount;
	m_nIndexSize = ( fmt == MATERIAL_INDEX_FORMAT_16BIT ) ? 2 : 4;
	m_nBufferSize = m_nIndexCount * m_nIndexSize;
	m_bLocked = false;
	m_pLockedData = NULL;
	
	Create();
}

CIndexBufferD3D8FF::~CIndexBufferD3D8FF()
{
	Destroy();
}

bool CIndexBufferD3D8FF::Create()
{
	if ( !Dx9Device() )
		return false;
	
	DWORD usage = D3DUSAGE_WRITEONLY;
	D3DPOOL pool = D3DPOOL_MANAGED;
	
	if ( IsDynamic() )
	{
		usage |= D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	}
	
	HRESULT hr = Dx9Device()->CreateIndexBuffer(
		m_nBufferSize,
		usage,
		m_d3dFormat,
		pool,
		&m_pIndexBuffer,
		NULL
	);
	
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to create index buffer (size=%d, format=%d)\n", m_nBufferSize, m_IndexFormat );
		return false;
	}
	
	return true;
}

void CIndexBufferD3D8FF::Destroy()
{
	if ( m_pIndexBuffer )
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
}

bool CIndexBufferD3D8FF::Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc )
{
	if ( !m_pIndexBuffer )
		return false;
	
	DWORD flags = 0;
	if ( bAppend )
		flags |= D3DLOCK_NOOVERWRITE;
	else if ( IsDynamic() )
		flags |= D3DLOCK_DISCARD;
	
	HRESULT hr = m_pIndexBuffer->Lock( 0, 0, &m_pLockedData, flags );
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to lock index buffer\n" );
		return false;
	}
	
	m_bLocked = true;
	
	// Fill in index desc
	desc.m_pIndices = (unsigned short*)m_pLockedData;
	desc.m_nIndexSize = m_nIndexSize;
	
	return true;
}

void CIndexBufferD3D8FF::Unlock( int nWrittenIndexCount, IndexDesc_t &desc )
{
	if ( !m_bLocked )
		return;
	
	m_pIndexBuffer->Unlock();
	m_bLocked = false;
	m_pLockedData = NULL;
}

//-----------------------------------------------------------------------------
// Mesh Implementation
//-----------------------------------------------------------------------------
CMeshD3D8FF::CMeshD3D8FF( const char *pTextureGroupName )
{
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_PrimitiveType = MATERIAL_TRIANGLES;
	m_VertexFormat = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2);
	m_bMeshLocked = false;
}

CMeshD3D8FF::~CMeshD3D8FF()
{
}

void CMeshD3D8FF::SetVertexBuffer( IVertexBuffer *pVertexBuffer )
{
	m_pVertexBuffer = static_cast<CVertexBufferD3D8FF*>( pVertexBuffer );
}

void CMeshD3D8FF::SetIndexBuffer( IIndexBuffer *pIndexBuffer )
{
	m_pIndexBuffer = static_cast<CIndexBufferD3D8FF*>( pIndexBuffer );
}

void CMeshD3D8FF::SetVertexFormat( VertexFormat_t format )
{
	m_VertexFormat = format;
}

void CMeshD3D8FF::LockMesh( int nVertexCount, int nIndexCount, MeshDesc_t &desc )
{
	if ( !m_pVertexBuffer || !m_pIndexBuffer )
		return;
	
	// Lock vertex buffer (MeshDesc_t inherits from VertexDesc_t)
	m_pVertexBuffer->Lock( nVertexCount, false, static_cast<VertexDesc_t&>(desc) );
	
	// Lock index buffer (MeshDesc_t inherits from IndexDesc_t)
	m_pIndexBuffer->Lock( nIndexCount, false, static_cast<IndexDesc_t&>(desc) );
	
	m_bMeshLocked = true;
}

void CMeshD3D8FF::UnlockMesh( int nVertexCount, int nIndexCount, MeshDesc_t &desc )
{
	if ( !m_bMeshLocked )
		return;
	
	// Unlock buffers (MeshDesc_t inherits from VertexDesc_t and IndexDesc_t)
	if ( m_pVertexBuffer )
		m_pVertexBuffer->Unlock( nVertexCount, static_cast<VertexDesc_t&>(desc) );
	
	if ( m_pIndexBuffer )
		m_pIndexBuffer->Unlock( nIndexCount, static_cast<IndexDesc_t&>(desc) );
	
	m_bMeshLocked = false;
}

bool CMeshD3D8FF::SetRenderState( int nVertexOffsetInBytes, int nFirstVertexIdx )
{
	if ( !Dx9Device() || !m_pVertexBuffer || !m_pIndexBuffer )
		return false;
	
	// Set vertex buffer
	HRESULT hr = Dx9Device()->SetStreamSource( 
		0, 
		m_pVertexBuffer->GetD3DVertexBuffer(), 
		nVertexOffsetInBytes,
		m_pVertexBuffer->GetVertexSize()
	);
	
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to set stream source\n" );
		return false;
	}
	
	// Set index buffer
	hr = Dx9Device()->SetIndices( m_pIndexBuffer->GetD3DIndexBuffer() );
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to set indices\n" );
		return false;
	}
	
	// Set FVF
	hr = Dx9Device()->SetFVF( m_pVertexBuffer->GetFVF() );
	if ( FAILED( hr ) )
	{
		Warning( "D3D8FF: Failed to set FVF\n" );
		return false;
	}
	
	return true;
}

void CMeshD3D8FF::Draw( int nFirstIndex, int nIndexCount )
{
	VPROF( "CMeshD3D8FF::Draw" );
	
	if ( nFirstIndex == -1 || nIndexCount == 0 )
	{
		nFirstIndex = 0;
		nIndexCount = m_pIndexBuffer ? m_pIndexBuffer->IndexCount() : 0;
	}
	
	CPrimList primList;
	primList.m_FirstIndex = nFirstIndex;
	primList.m_NumIndices = nIndexCount;
	
	Draw( &primList, 1 );
}

void CMeshD3D8FF::Draw( CPrimList *pLists, int nLists )
{
	VPROF( "CMeshD3D8FF::Draw(lists)" );
	
	if ( !pLists || nLists == 0 )
		return;
	
	// Make sure there's something to draw
	int i;
	for ( i = 0; i < nLists; ++i )
	{
		if ( pLists[i].m_NumIndices > 0 )
			break;
	}
	
	if ( i == nLists )
		return;
	
	// Set render state
	if ( !SetRenderState( 0, 0 ) )
		return;
	
	// Draw all primitive lists
	DrawInternal( pLists, nLists );
}

void CMeshD3D8FF::DrawInternal( CPrimList *pLists, int nLists )
{
	if ( !Dx9Device() || !m_pVertexBuffer || !m_pIndexBuffer )
		return;
	
	// Convert primitive type
	D3DPRIMITIVETYPE d3dPrimType = D3DPT_TRIANGLELIST;
	switch ( m_PrimitiveType )
	{
	case MATERIAL_POINTS:
		d3dPrimType = D3DPT_POINTLIST;
		break;
	case MATERIAL_LINES:
		d3dPrimType = D3DPT_LINELIST;
		break;
	case MATERIAL_TRIANGLES:
		d3dPrimType = D3DPT_TRIANGLELIST;
		break;
	case MATERIAL_TRIANGLE_STRIP:
		d3dPrimType = D3DPT_TRIANGLESTRIP;
		break;
	case MATERIAL_LINE_STRIP:
		d3dPrimType = D3DPT_LINESTRIP;
		break;
	default:
		Warning( "D3D8FF: Unsupported primitive type %d\n", m_PrimitiveType );
		return;
	}
	
	// Draw each primitive list
	int nVertexCount = m_pVertexBuffer->VertexCount();
	for ( int i = 0; i < nLists; ++i )
	{
		if ( pLists[i].m_NumIndices == 0 )
			continue;
		
		int numPrimitives = 0;
		switch ( m_PrimitiveType )
		{
		case MATERIAL_POINTS:
			numPrimitives = pLists[i].m_NumIndices;
			break;
		case MATERIAL_LINES:
			numPrimitives = pLists[i].m_NumIndices / 2;
			break;
		case MATERIAL_TRIANGLES:
			numPrimitives = pLists[i].m_NumIndices / 3;
			break;
		case MATERIAL_TRIANGLE_STRIP:
			numPrimitives = pLists[i].m_NumIndices - 2;
			break;
		case MATERIAL_LINE_STRIP:
			numPrimitives = pLists[i].m_NumIndices - 1;
			break;
		}
		
		if ( numPrimitives <= 0 )
			continue;
		
		// Draw indexed primitives
		HRESULT hr = Dx9Device()->DrawIndexedPrimitive(
			d3dPrimType,
			0,  // BaseVertexIndex
			0,  // MinVertexIndex
			nVertexCount,
			pLists[i].m_FirstIndex,
			numPrimitives
		);
		
		if ( FAILED( hr ) )
		{
			Warning( "D3D8FF: DrawIndexedPrimitive failed (hr=0x%X)\n", hr );
		}
	}
}

//-----------------------------------------------------------------------------
// Mesh Manager Implementation
//-----------------------------------------------------------------------------
CMeshMgrD3D8FF::CMeshMgrD3D8FF()
{
	m_pDynamicVertexBuffer = NULL;
	m_pDynamicIndexBuffer = NULL;
	m_pDynamicMesh = NULL;
}

CMeshMgrD3D8FF::~CMeshMgrD3D8FF()
{
}

void CMeshMgrD3D8FF::Init()
{
	Msg( "D3D8FF: Initializing mesh manager\n" );
	
	// Create dynamic buffers
	m_pDynamicVertexBuffer = new CVertexBufferD3D8FF(
		SHADER_BUFFER_TYPE_DYNAMIC,
		VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2),
		DYNAMIC_VERTEX_BUFFER_SIZE,
		"dynamic"
	);
	
	m_pDynamicIndexBuffer = new CIndexBufferD3D8FF(
		SHADER_BUFFER_TYPE_DYNAMIC,
		MATERIAL_INDEX_FORMAT_16BIT,
		DYNAMIC_INDEX_BUFFER_SIZE,
		"dynamic"
	);
	
	// Create dynamic mesh
	m_pDynamicMesh = new CMeshD3D8FF( "dynamic" );
	m_pDynamicMesh->SetVertexBuffer( m_pDynamicVertexBuffer );
	m_pDynamicMesh->SetIndexBuffer( m_pDynamicIndexBuffer );
}

void CMeshMgrD3D8FF::Shutdown()
{
	Msg( "D3D8FF: Shutting down mesh manager\n" );
	
	// Destroy dynamic mesh
	if ( m_pDynamicMesh )
	{
		delete m_pDynamicMesh;
		m_pDynamicMesh = NULL;
	}
	
	// Destroy dynamic buffers
	if ( m_pDynamicVertexBuffer )
	{
		delete m_pDynamicVertexBuffer;
		m_pDynamicVertexBuffer = NULL;
	}
	
	if ( m_pDynamicIndexBuffer )
	{
		delete m_pDynamicIndexBuffer;
		m_pDynamicIndexBuffer = NULL;
	}
	
	// Destroy all static meshes
	for ( int i = 0; i < m_StaticMeshes.Count(); ++i )
	{
		delete m_StaticMeshes[i];
	}
	m_StaticMeshes.RemoveAll();
	
	// Destroy all buffers
	for ( int i = 0; i < m_VertexBuffers.Count(); ++i )
	{
		delete m_VertexBuffers[i];
	}
	m_VertexBuffers.RemoveAll();
	
	for ( int i = 0; i < m_IndexBuffers.Count(); ++i )
	{
		delete m_IndexBuffers[i];
	}
	m_IndexBuffers.RemoveAll();
}

IMesh* CMeshMgrD3D8FF::GetDynamicMesh( IMaterial* pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount,
	bool bBuffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	// Return the dynamic mesh
	if ( m_pDynamicMesh )
	{
		m_pDynamicMesh->SetVertexFormat( vertexFormat );
	}
	
	return m_pDynamicMesh;
}

IVertexBuffer* CMeshMgrD3D8FF::CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup )
{
	CVertexBufferD3D8FF *pBuffer = new CVertexBufferD3D8FF( type, fmt, nVertexCount, pBudgetGroup );
	m_VertexBuffers.AddToTail( pBuffer );
	return pBuffer;
}

void CMeshMgrD3D8FF::DestroyVertexBuffer( IVertexBuffer *pVertexBuffer )
{
	CVertexBufferD3D8FF *pBuffer = static_cast<CVertexBufferD3D8FF*>( pVertexBuffer );
	m_VertexBuffers.FindAndRemove( pBuffer );
	delete pBuffer;
}

IIndexBuffer* CMeshMgrD3D8FF::CreateIndexBuffer( ShaderBufferType_t type, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup )
{
	CIndexBufferD3D8FF *pBuffer = new CIndexBufferD3D8FF( type, fmt, nIndexCount, pBudgetGroup );
	m_IndexBuffers.AddToTail( pBuffer );
	return pBuffer;
}

void CMeshMgrD3D8FF::DestroyIndexBuffer( IIndexBuffer *pIndexBuffer )
{
	CIndexBufferD3D8FF *pBuffer = static_cast<CIndexBufferD3D8FF*>( pIndexBuffer );
	m_IndexBuffers.FindAndRemove( pBuffer );
	delete pBuffer;
}

IMesh* CMeshMgrD3D8FF::CreateStaticMesh( VertexFormat_t format, const char *pBudgetGroup, IMaterial *pMaterial )
{
	CMeshD3D8FF *pMesh = new CMeshD3D8FF( pBudgetGroup );
	pMesh->SetVertexFormat( format );
	m_StaticMeshes.AddToTail( pMesh );
	return pMesh;
}

void CMeshMgrD3D8FF::DestroyStaticMesh( IMesh *pMesh )
{
	CMeshD3D8FF *pMeshD3D8FF = static_cast<CMeshD3D8FF*>( pMesh );
	m_StaticMeshes.FindAndRemove( pMeshD3D8FF );
	delete pMeshD3D8FF;
}

IVertexBuffer* CMeshMgrD3D8FF::GetDynamicVertexBuffer( int nStreamID, VertexFormat_t vertexFormat )
{
	return m_pDynamicVertexBuffer;
}

IIndexBuffer* CMeshMgrD3D8FF::GetDynamicIndexBuffer()
{
	return m_pDynamicIndexBuffer;
}

void CMeshMgrD3D8FF::Flush()
{
	// Nothing to flush in fixed function
}

//-----------------------------------------------------------------------------
// IVertexBuffer methods for CMeshD3D8FF
//-----------------------------------------------------------------------------
bool CMeshD3D8FF::Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc )
{
	// Delegate to vertex buffer if we have one
	if ( m_pVertexBuffer )
	{
		return m_pVertexBuffer->Lock( nVertexCount, bAppend, desc );
	}
	return false;
}

void CMeshD3D8FF::Unlock( int nWrittenVertexCount, VertexDesc_t &desc )
{
	// Delegate to vertex buffer
	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Unlock( nWrittenVertexCount, desc );
	}
}

// IIndexBuffer implementation for CMeshD3D8FF
bool CMeshD3D8FF::Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc )
{
	if ( !m_pIndexBuffer )
		return false;
	
	return m_pIndexBuffer->Lock( nMaxIndexCount, bAppend, desc );
}

void CMeshD3D8FF::Unlock( int nWrittenIndexCount, IndexDesc_t &desc )
{
	if ( m_pIndexBuffer )
	{
		m_pIndexBuffer->Unlock( nWrittenIndexCount, desc );
	}
}

