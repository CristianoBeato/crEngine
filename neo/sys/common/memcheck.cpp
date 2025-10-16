#include "precompiled.h"
#include "sys/sys_local.h"
#include "sys/sys_public.h"

#if __PLATFORM_WINDOWS__

#pragma optimize( "", on )

#ifdef DEBUG 

static unsigned int debug_total_alloc = 0;
static unsigned int debug_total_alloc_count = 0;
static unsigned int debug_current_alloc = 0;
static unsigned int debug_current_alloc_count = 0;
static unsigned int debug_frame_alloc = 0;
static unsigned int debug_frame_alloc_count = 0;

idCVar sys_showMallocs( "sys_showMallocs", "0", CVAR_SYSTEM, "" );

// _HOOK_ALLOC, _HOOK_REALLOC, _HOOK_FREE

typedef struct CrtMemBlockHeader
{
	struct _CrtMemBlockHeader *pBlockHeaderNext;	// Pointer to the block allocated just before this one:
	struct _CrtMemBlockHeader *pBlockHeaderPrev;	// Pointer to the block allocated just after this one
   char *szFileName;    // File name
   int nLine;           // Line number
   size_t nDataSize;    // Size of user block
   int nBlockUse;       // Type of block
   long lRequest;       // Allocation number
	byte		gap[4];								// Buffer just before (lower than) the user's memory:
} CrtMemBlockHeader;

#include <crtdbg.h>

/*
==================
Sys_AllocHook

	called for every malloc/new/free/delete
==================
*/
int Sys_AllocHook( int nAllocType, void *pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char * szFileName, int nLine ) 
{
	CrtMemBlockHeader	*pHead;
	byte				*temp;

	if ( nBlockUse == _CRT_BLOCK )
	{
      return( TRUE );
	}

	// get a pointer to memory block header
	temp = ( byte * )pvData;
	temp -= 32;
	pHead = ( CrtMemBlockHeader * )temp;

	switch( nAllocType ) {
		case	_HOOK_ALLOC:
			debug_total_alloc += nSize;
			debug_current_alloc += nSize;
			debug_frame_alloc += nSize;
			debug_total_alloc_count++;
			debug_current_alloc_count++;
			debug_frame_alloc_count++;
			break;

		case	_HOOK_FREE:
			assert( pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd );

			debug_current_alloc -= pHead->nDataSize;
			debug_current_alloc_count--;
			debug_total_alloc_count++;
			debug_frame_alloc_count++;
			break;

		case	_HOOK_REALLOC:
			assert( pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd );

			debug_current_alloc -= pHead->nDataSize;
			debug_total_alloc += nSize;
			debug_current_alloc += nSize;
			debug_frame_alloc += nSize;
			debug_total_alloc_count++;
			debug_current_alloc_count--;
			debug_frame_alloc_count++;
			break;
	}
	return( TRUE );
}

/*
==================
Sys_DebugMemory_f
==================
*/
void Sys_DebugMemory_f() 
{
  	common->Printf( "Total allocation %8dk in %d blocks\n", debug_total_alloc / 1024, debug_total_alloc_count );
  	common->Printf( "Current allocation %8dk in %d blocks\n", debug_current_alloc / 1024, debug_current_alloc_count );
}

/*
==================
Sys_MemFrame
==================
*/
void Sys_MemFrame() 
{
	if( sys_showMallocs.GetInteger() ) {
		common->Printf("Frame: %8dk in %5d blocks\n", debug_frame_alloc / 1024, debug_frame_alloc_count );
	}

	debug_frame_alloc = 0;
	debug_frame_alloc_count = 0;
}

#endif

#elif __PLATFORM_LINUX__
#ifdef ID_MCHECK
#include <mcheck.h>
#endif


/*
===============
mem consistency stuff
===============
*/

#ifdef ID_MCHECK

const char* mcheckstrings[] =
{
	"MCHECK_DISABLED",
	"MCHECK_OK",
	"MCHECK_FREE",	// block freed twice
	"MCHECK_HEAD",	// memory before the block was clobbered
	"MCHECK_TAIL"	// memory after the block was clobbered
};

void abrt_func( mcheck_status status )
{
	Sys_Printf( "memory consistency failure: %s\n", mcheckstrings[ status + 1 ] );
	Posix_SetExit( EXIT_FAILURE );
	common->Quit();
}

#endif //ID_MCHECK
#endif //__PLATFORM_LINUX__

