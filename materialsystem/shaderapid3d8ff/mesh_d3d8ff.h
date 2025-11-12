//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: D3D8FF Mesh System
//
//===========================================================================//

#ifndef MESH_D3D8FF_H
#define MESH_D3D8FF_H

#ifdef _WIN32
#pragma once
#endif

#include "locald3dtypes.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "shaderapi/ishaderapi.h"
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CVertexBufferD3D8FF;
class CIndexBufferD3D8FF;
class CMeshD3D8FF;
struct ICachedPerFrameMeshData;

//-----------------------------------------------------------------------------
// Vertex buffer for D3D8FF
//-----------------------------------------------------------------------------
class CVertexBufferD3D8FF : public IVertexBuffer
{
public:
	CVertexBufferD3D8FF( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup );
	virtual ~CVertexBufferD3D8FF();

	// IVertexBuffer implementation
	virtual int VertexCount() const { return m_nVertexCount; }
	virtual VertexFormat_t GetVertexFormat() const { return m_VertexFormat; }
	virtual bool IsDynamic() const { return IsDynamicBufferType( m_Type ); }
	virtual void BeginCastBuffer( VertexFormat_t format ) {}
	virtual void EndCastBuffer() {}
	virtual int GetRoomRemaining() const { return m_nVertexCount; }

	// Lock/unlock for writing
	virtual bool Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc );
	virtual void Unlock( int nVertexCount, VertexDesc_t &desc );

	// Spew the mesh data
	virtual void Spew( int nVertexCount, const VertexDesc_t &desc ) {}
	virtual void ValidateData( int nVertexCount, const VertexDesc_t &desc ) {}

	// D3D9 specific
	IDirect3DVertexBuffer9* GetD3DVertexBuffer() { return m_pVertexBuffer; }
	DWORD GetFVF() const { return m_dwFVF; }
	int GetVertexSize() const { return m_nVertexSize; }

private:
	bool Create();
	void Destroy();
	DWORD ComputeFVF( VertexFormat_t format );

	IDirect3DVertexBuffer9 *m_pVertexBuffer;
	ShaderBufferType_t m_Type;
	VertexFormat_t m_VertexFormat;
	DWORD m_dwFVF;
	int m_nVertexCount;
	int m_nVertexSize;
	int m_nBufferSize;
	bool m_bLocked;
	void *m_pLockedData;
};

//-----------------------------------------------------------------------------
// Index buffer for D3D8FF
//-----------------------------------------------------------------------------
class CIndexBufferD3D8FF : public IIndexBuffer
{
public:
	CIndexBufferD3D8FF( ShaderBufferType_t type, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup );
	virtual ~CIndexBufferD3D8FF();

	// IIndexBuffer implementation
	virtual int IndexCount() const { return m_nIndexCount; }
	virtual MaterialIndexFormat_t IndexFormat() const { return m_IndexFormat; }
	virtual bool IsDynamic() const { return IsDynamicBufferType( m_Type ); }
	virtual void BeginCastBuffer( MaterialIndexFormat_t format ) {}
	virtual void EndCastBuffer() {}
	virtual int GetRoomRemaining() const { return m_nIndexCount; }

	// Lock/unlock for writing
	virtual bool Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc );
	virtual void Unlock( int nWrittenIndexCount, IndexDesc_t &desc );

	// Modify the index buffer
	virtual void ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc ) {}
	virtual void ModifyEnd( IndexDesc_t &desc ) {}

	// Spew the mesh data
	virtual void Spew( int nIndexCount, const IndexDesc_t &desc ) {}
	virtual void ValidateData( int nIndexCount, const IndexDesc_t &desc ) {}

	// D3D9 specific
	IDirect3DIndexBuffer9* GetD3DIndexBuffer() { return m_pIndexBuffer; }
	D3DFORMAT GetD3DFormat() const { return m_d3dFormat; }

private:
	bool Create();
	void Destroy();

	IDirect3DIndexBuffer9 *m_pIndexBuffer;
	ShaderBufferType_t m_Type;
	MaterialIndexFormat_t m_IndexFormat;
	D3DFORMAT m_d3dFormat;
	int m_nIndexCount;
	int m_nIndexSize;
	int m_nBufferSize;
	bool m_bLocked;
	void *m_pLockedData;
};

//-----------------------------------------------------------------------------
// Mesh for D3D8FF
//-----------------------------------------------------------------------------
class CMeshD3D8FF : public IMesh
{
public:
	CMeshD3D8FF( const char *pTextureGroupName );
	virtual ~CMeshD3D8FF();

	// IMesh implementation
	virtual void SetPrimitiveType( MaterialPrimitiveType_t type ) { m_PrimitiveType = type; }
	virtual void Draw( int nFirstIndex = -1, int nIndexCount = 0 );
	virtual void SetColorMesh( IMesh *pColorMesh, int nVertexOffset ) {}
	virtual void Draw( CPrimList *pLists, int nLists );
	virtual void CopyToMeshBuilder( int iStartVert, int nVerts, int iStartIndex, int nIndices, int indexOffset, CMeshBuilder &builder ) {}

	// Vertex/index buffer access
	virtual void SetVertexBuffer( IVertexBuffer *pVertexBuffer );
	virtual void SetIndexBuffer( IIndexBuffer *pIndexBuffer );

	// Sets/gets the vertex format
	virtual void SetVertexFormat( VertexFormat_t format );
	virtual VertexFormat_t GetVertexFormat() const { return m_VertexFormat; }

	// Lock/unlock
	virtual void LockMesh( int nVertexCount, int nIndexCount, MeshDesc_t &desc );
	virtual void ModifyBegin( int nFirstVertex, int nVertexCount, int nFirstIndex, int nIndexCount, MeshDesc_t &desc ) {}
	virtual void ModifyEnd( MeshDesc_t &desc ) {}
	virtual void UnlockMesh( int nVertexCount, int nIndexCount, MeshDesc_t &desc );

	// Spew data
	virtual void Spew( int nVertexCount, int nIndexCount, const MeshDesc_t &desc ) {}
	virtual void ValidateData( int nVertexCount, int nIndexCount, const MeshDesc_t &desc ) {}

	// IVertexBuffer implementation
	virtual int VertexCount() const { return m_pVertexBuffer ? m_pVertexBuffer->VertexCount() : 0; }
	virtual bool IsDynamic() const { return m_pVertexBuffer ? m_pVertexBuffer->IsDynamic() : false; }
	virtual void BeginCastBuffer( VertexFormat_t format ) {}
	virtual void EndCastBuffer() {}
	virtual int GetRoomRemaining() const { return 0; }
	virtual bool Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc );
	virtual void Unlock( int nWrittenVertexCount, VertexDesc_t &desc );
	virtual void Spew( int nVertexCount, const VertexDesc_t &desc ) {}
	virtual void ValidateData( int nVertexCount, const VertexDesc_t &desc ) {}
	
	// IIndexBuffer implementation
	virtual int IndexCount() const { return m_pIndexBuffer ? m_pIndexBuffer->IndexCount() : 0; }
	virtual MaterialIndexFormat_t IndexFormat() const { return m_pIndexBuffer ? m_pIndexBuffer->IndexFormat() : MATERIAL_INDEX_FORMAT_16BIT; }
	virtual void BeginCastBuffer( MaterialIndexFormat_t format ) {}
	virtual bool Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc );
	virtual void Unlock( int nWrittenIndexCount, IndexDesc_t &desc );
	virtual void ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc ) {}
	virtual void ModifyEnd( IndexDesc_t &desc ) {}
	virtual void Spew( int nIndexCount, const IndexDesc_t &desc ) {}
	virtual void ValidateData( int nIndexCount, const IndexDesc_t &desc ) {}
	
	// IMesh stubs
	virtual void ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t &desc ) {}
	virtual void SetFlexMesh( IMesh *pMesh, int nVertexOffset ) {}
	virtual void DisableFlexMesh() {}
	virtual void MarkAsDrawn() {}
	virtual void DrawModulated( const Vector4D &diffuseModulation, int nFirstIndex = -1, int nIndexCount = 0 ) { Draw( nFirstIndex, nIndexCount ); }
	virtual unsigned int ComputeMemoryUsed() { return 0; }
	virtual void *AccessRawHardwareDataStream( uint8 nRawStreamIndex, uint32 numBytes, uint32 uiFlags, void *pvContext ) { return NULL; }
	virtual ICachedPerFrameMeshData *GetCachedPerFrameMeshData() { return NULL; }
	virtual void ReconstructFromCachedPerFrameMeshData( ICachedPerFrameMeshData *pData ) {}

	// Additional methods for actual drawing
	bool SetRenderState( int nVertexOffsetInBytes, int nFirstVertexIdx );
	void DrawInternal( CPrimList *pLists, int nLists );

private:
	CVertexBufferD3D8FF *m_pVertexBuffer;
	CIndexBufferD3D8FF *m_pIndexBuffer;
	MaterialPrimitiveType_t m_PrimitiveType;
	VertexFormat_t m_VertexFormat;
	bool m_bMeshLocked;
};

//-----------------------------------------------------------------------------
// Mesh manager for D3D8FF
//-----------------------------------------------------------------------------
class CMeshMgrD3D8FF
{
public:
	CMeshMgrD3D8FF();
	~CMeshMgrD3D8FF();

	void Init();
	void Shutdown();

	// Dynamic mesh
	IMesh* GetDynamicMesh( IMaterial* pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount,
		bool bBuffered, IMesh* pVertexOverride, IMesh* pIndexOverride );

	// Vertex/index buffer creation
	IVertexBuffer* CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup );
	void DestroyVertexBuffer( IVertexBuffer *pVertexBuffer );

	IIndexBuffer* CreateIndexBuffer( ShaderBufferType_t type, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup );
	void DestroyIndexBuffer( IIndexBuffer *pIndexBuffer );

	// Static mesh
	IMesh* CreateStaticMesh( VertexFormat_t format, const char *pBudgetGroup, IMaterial *pMaterial );
	void DestroyStaticMesh( IMesh *pMesh );

	// Get dynamic buffers
	IVertexBuffer* GetDynamicVertexBuffer( int nStreamID, VertexFormat_t vertexFormat );
	IIndexBuffer* GetDynamicIndexBuffer();

	// Flush any buffered primitives
	void Flush();

private:
	// Dynamic buffers (reused each frame)
	CVertexBufferD3D8FF *m_pDynamicVertexBuffer;
	CIndexBufferD3D8FF *m_pDynamicIndexBuffer;

	// Dynamic mesh
	CMeshD3D8FF *m_pDynamicMesh;

	// Lists of created buffers
	CUtlVector<CVertexBufferD3D8FF*> m_VertexBuffers;
	CUtlVector<CIndexBufferD3D8FF*> m_IndexBuffers;
	CUtlVector<CMeshD3D8FF*> m_StaticMeshes;

	// Dynamic buffer sizes
	static const int DYNAMIC_VERTEX_BUFFER_SIZE = 32768;
	static const int DYNAMIC_INDEX_BUFFER_SIZE = 32768;
};

// Global mesh manager
extern CMeshMgrD3D8FF g_MeshMgr;

#endif // MESH_D3D8FF_H

