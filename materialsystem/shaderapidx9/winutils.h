//========= Copyright Valve Corporation, All rights reserved. ============//
//
//
//
//==================================================================================================
#ifndef WINUTILS_H
#define WINUTILS_H

// Include appropriate header for Windows types based on backend
#if defined( USE_DXVK_NATIVE )
	#include "locald3dtypes.h"  // DXVK provides Windows types
#else
	#include "togl/rendermechanism.h" // ToGL provides Windows types
#endif

#if !defined(_WIN32)

	void Sleep( unsigned int ms );
	bool IsIconic( VD3DHWND hWnd );
	BOOL ClientToScreen( VD3DHWND hWnd, LPPOINT pPoint );
	void* GetCurrentThread();
	void SetThreadAffinityMask( void *hThread, int nMask );
	void GlobalMemoryStatus( MEMORYSTATUS *pOut );
#endif

#endif // WINUTILS_H
