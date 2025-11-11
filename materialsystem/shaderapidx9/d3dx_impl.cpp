// D3DX Math Library Implementation for DXVK-native
// Provides implementations for D3DX math functions that DXVK doesn't include

#include "locald3dtypes.h"
#include <math.h>
#include <string.h>

// Matrix Identity
D3DXMATRIX* WINAPI D3DXMatrixIdentity(D3DXMATRIX *pOut) {
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.0f;
    return pOut;
}

// Matrix Transpose
D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM) {
    D3DXMATRIX temp;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp.m[i][j] = pM->m[j][i];
        }
    }
    *pOut = temp;
    return pOut;
}

// Matrix Multiply
D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2) {
    D3DXMATRIX temp;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                temp.m[i][j] += pM1->m[i][k] * pM2->m[k][j];
            }
        }
    }
    *pOut = temp;
    return pOut;
}

// Matrix Inverse
D3DXMATRIX* WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM) {
    // Simplified 4x4 matrix inversion using Gauss-Jordan elimination
    float m[4][4], inv[4][4];
    memcpy(m, pM, sizeof(m));
   
    // Initialize inverse as identity
    memset(inv, 0, sizeof(inv));
    for (int i = 0; i < 4; i++) inv[i][i] = 1.0f;
   
    // Gauss-Jordan elimination
    for (int i = 0; i < 4; i++) {
        float pivot = m[i][i];
        if (fabs(pivot) < 0.000001f) {
            if (pDeterminant) *pDeterminant = 0.0f;
            return D3DXMatrixIdentity(pOut);
        }
       
        for (int j = 0; j < 4; j++) {
            m[i][j] /= pivot;
            inv[i][j] /= pivot;
        }
       
        for (int k = 0; k < 4; k++) {
            if (k != i) {
                float factor = m[k][i];
                for (int j = 0; j < 4; j++) {
                    m[k][j] -= factor * m[i][j];
                    inv[k][j] -= factor * inv[i][j];
                }
            }
        }
    }
   
    memcpy(pOut, inv, sizeof(inv));
    if (pDeterminant) *pDeterminant = 1.0f;
    return pOut;
}

// Matrix Translation
D3DXMATRIX* WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z) {
    D3DXMatrixIdentity(pOut);
    pOut->_41 = x;
    pOut->_42 = y;
    pOut->_43 = z;
    return pOut;
}

// Matrix Perspective RH
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveRH(D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf) {
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f * zn / w;
    pOut->_22 = 2.0f * zn / h;
    pOut->_33 = zf / (zn - zf);
    pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}

// Matrix Perspective Off-Center RH
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterRH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf) {
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f * zn / (r - l);
    pOut->_22 = 2.0f * zn / (t - b);
    pOut->_31 = (l + r) / (l - r);
    pOut->_32 = (t + b) / (b - t);
    pOut->_33 = zf / (zn - zf);
    pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}

// Matrix Ortho Off-Center RH
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf) {
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f / (r - l);
    pOut->_22 = 2.0f / (t - b);
    pOut->_33 = 1.0f / (zn - zf);
    pOut->_41 = (l + r) / (l - r);
    pOut->_42 = (t + b) / (b - t);
    pOut->_43 = zn / (zn - zf);
    pOut->_44 = 1.0f;
    return pOut;
}

// Plane Transform
D3DXPLANE* WINAPI D3DXPlaneTransform(D3DXPLANE *pOut, CONST D3DXPLANE *pP, CONST D3DXMATRIX *pM) {
    D3DXPLANE temp;
    temp.a = pP->a * pM->_11 + pP->b * pM->_21 + pP->c * pM->_31 + pP->d * pM->_41;
    temp.b = pP->a * pM->_12 + pP->b * pM->_22 + pP->c * pM->_32 + pP->d * pM->_42;
    temp.c = pP->a * pM->_13 + pP->b * pM->_23 + pP->c * pM->_33 + pP->d * pM->_43;
    temp.d = pP->a * pM->_14 + pP->b * pM->_24 + pP->c * pM->_34 + pP->d * pM->_44;
    *pOut = temp;
    return pOut;
}

// Plane Normalize
D3DXPLANE* WINAPI D3DXPlaneNormalize(D3DXPLANE *pOut, CONST D3DXPLANE *pP) {
    float len = sqrtf(pP->a * pP->a + pP->b * pP->b + pP->c * pP->c);
    if (len > 0.0f) {
        pOut->a = pP->a / len;
        pOut->b = pP->b / len;
        pOut->c = pP->c / len;
        pOut->d = pP->d / len;
    } else {
        *pOut = *pP;
    }
    return pOut;
}

// Vector3 functions
D3DXVECTOR3* WINAPI D3DXVec3Normalize(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV) {
    float len = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
    if (len > 0.0f) {
        pOut->x = pV->x / len;
        pOut->y = pV->y / len;
        pOut->z = pV->z / len;
    } else {
        *pOut = *pV;
    }
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Subtract(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2) {
    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Cross(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2) {
    D3DXVECTOR3 temp;
    temp.x = pV1->y * pV2->z - pV1->z * pV2->y;
    temp.y = pV1->z * pV2->x - pV1->x * pV2->z;
    temp.z = pV1->x * pV2->y - pV1->y * pV2->x;
    *pOut = temp;
    return pOut;
}

FLOAT WINAPI D3DXVec3Dot(CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2) {
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

FLOAT WINAPI D3DXVec3Length(CONST D3DXVECTOR3 *pV) {
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

D3DXVECTOR3* WINAPI D3DXVec3Transform(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM) {
    D3DXVECTOR3 temp;
    temp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
    temp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
    temp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;
    *pOut = temp;
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM) {
    float w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pM->_44;
    D3DXVECTOR3 temp;
    temp.x = (pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41) / w;
    temp.y = (pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42) / w;
    temp.z = (pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43) / w;
    *pOut = temp;
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformNormal(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM) {
    D3DXVECTOR3 temp;
    temp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31;
    temp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32;
    temp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33;
    *pOut = temp;
    return pOut;
}

// Vector4 functions
D3DXVECTOR4* WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM) {
    D3DXVECTOR4 temp;
    temp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pV->w * pM->_41;
    temp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pV->w * pM->_42;
    temp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pV->w * pM->_43;
    temp.w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pV->w * pM->_44;
    *pOut = temp;
    return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Normalize(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV) {
    float len = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
    if (len > 0.0f) {
        pOut->x = pV->x / len;
        pOut->y = pV->y / len;
        pOut->z = pV->z / len;
        pOut->w = pV->w / len;
    } else {
        *pOut = *pV;
    }
    return pOut;
}

// Matrix Stack Implementation
class CMatrixStack : public ID3DXMatrixStack {
private:
    D3DXMATRIX *m_pStack;
    int m_nSize;
    int m_nCapacity;
    ULONG m_nRefCount;

public:
    CMatrixStack(DWORD flags) : m_nSize(0), m_nCapacity(16), m_nRefCount(1) {
        m_pStack = new D3DXMATRIX[m_nCapacity];
        D3DXMatrixIdentity(&m_pStack[0]);
        m_nSize = 1;
    }
   
    virtual ~CMatrixStack() {
        delete[] m_pStack;
    }
   
    HRESULT WINAPI QueryInterface(const IID&, void**) { return E_NOINTERFACE; }
    ULONG WINAPI AddRef() { return ++m_nRefCount; }
    ULONG WINAPI Release() {
        if (--m_nRefCount == 0) {
            delete this;
            return 0;
        }
        return m_nRefCount;
    }
   
    HRESULT WINAPI Pop() {
        if (m_nSize > 1) m_nSize--;
        return S_OK;
    }
   
    HRESULT WINAPI Push() {
        if (m_nSize >= m_nCapacity) {
            m_nCapacity *= 2;
            D3DXMATRIX *pNew = new D3DXMATRIX[m_nCapacity];
            memcpy(pNew, m_pStack, m_nSize * sizeof(D3DXMATRIX));
            delete[] m_pStack;
            m_pStack = pNew;
        }
        m_pStack[m_nSize] = m_pStack[m_nSize - 1];
        m_nSize++;
        return S_OK;
    }
   
    HRESULT WINAPI LoadIdentity() {
        D3DXMatrixIdentity(&m_pStack[m_nSize - 1]);
        return S_OK;
    }
   
    HRESULT WINAPI LoadMatrix(CONST D3DMATRIX *pM) {
        m_pStack[m_nSize - 1] = *(D3DXMATRIX*)pM;
        return S_OK;
    }
   
    HRESULT WINAPI MultMatrix(CONST D3DMATRIX *pM) {
        D3DXMatrixMultiply(&m_pStack[m_nSize - 1], &m_pStack[m_nSize - 1], (D3DXMATRIX*)pM);
        return S_OK;
    }
   
    HRESULT WINAPI MultMatrixLocal(CONST D3DMATRIX *pM) {
        D3DXMatrixMultiply(&m_pStack[m_nSize - 1], (D3DXMATRIX*)pM, &m_pStack[m_nSize - 1]);
        return S_OK;
    }
   
    HRESULT WINAPI RotateAxis(CONST D3DXVECTOR3*, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI RotateAxisLocal(CONST D3DXVECTOR3*, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI RotateYawPitchRoll(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI RotateYawPitchRollLocal(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI Scale(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI ScaleLocal(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI Translate(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
    HRESULT WINAPI TranslateLocal(FLOAT, FLOAT, FLOAT) { return E_NOTIMPL; }
   
    D3DXMATRIX* WINAPI GetTop() {
        return &m_pStack[m_nSize - 1];
    }
};

HRESULT WINAPI D3DXCreateMatrixStack(DWORD flags, ID3DXMatrixStack **ppStack) {
    *ppStack = new CMatrixStack(flags);
    return S_OK;
}

// Shader compilation stubs (DXVK handles shader compilation internally)
HRESULT WINAPI D3DXCompileShader(
    LPCSTR pSrcData,
    UINT SrcDataLen,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    LPCSTR pFunctionName,
    LPCSTR pProfile,
    DWORD Flags,
    LPD3DXBUFFER* ppShader,
    LPD3DXBUFFER* ppErrorMsgs,
    LPD3DXCONSTANTTABLE* ppConstantTable)
{
    // DXVK handles shader compilation through its D3D9 implementation
    // This stub should never be called in practice
    return E_NOTIMPL;
}

DWORD WINAPI D3DXGetShaderVersion(CONST DWORD* pFunction) {
    if (!pFunction) return 0;
    return *pFunction & 0xFFFF;  // Return shader version from bytecode header
}

// GetVidMemBytes stub
int GetVidMemBytes() {
    return 256 * 1024 * 1024; // Return 256MB as default
}

