
#include "precompiled.h"

#include <bitset>
#include <cfenv>
#include <cstdint>

#include <immintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>

#include <SDL3/SDL_cpuinfo.h>

#if defined(_MSC_VER)
#   include <float.h>
#   include <intrin.h>

// GCC convention
inline unsigned int __get_cpuid( unsigned int level, unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx )
{
	int regs[4] = { -1, -1, -1 ,-1 };
	__cpuid( regs, (int)level );
	*reinterpret_cast<int*>(eax) = regs[_REG_EAX];
	*reinterpret_cast<int*>(ebx) = regs[_REG_EBX];
	*reinterpret_cast<int*>(ecx) = regs[_REG_ECX];
	*reinterpret_cast<int*>(edx) = regs[_REG_EDX];
	return 1;
}

#else
#include <cpuid.h>
#endif

#ifdef strcmp
#undef strcmp
#endif // 

enum register_t : uint8_t 
{
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX
};

enum flags_t : uint32_t
{
    CPU_VENDOR_BIT  = 0x00000000,	// CPU Brand Bit
    CPU_BRAND_BIT   = 0x80000000,  // CPU Model Bit
    BIT_SSE3        = ( 1 << 0 ),	// bit 0 of ECX denotes SSE3 existence
    BIT_CMOV        = ( 1 << 15 ), // bit 15 of EDX denotes CMOV existence
    BIT_MMX         = ( 1 << 23 ), // bit 23 of EDX denotes MMX existence
    BIT_FXSAVE      = ( 1 << 24 ), // bit 24 of EDX denotes support for FXSAVE
    BIT_SSE         = ( 1 << 25 ), // bit 25 of EDX denotes SSE existence
    BIT_SSE2        = ( 1 << 26 ), // bit 26 of EDX denotes SSE2 existence
    BIT_HTT         = ( 1 << 28 ), // bit 28 of EDX denotes HTT existence
    BIT_3DNOW       = ( 1 << 31 ) // bit 31 of EDX denotes 3DNow! support
};

static uint32_t nIds = 0;
static uint32_t nExIds = 0;
static uint32_t cpui[4] = { 0, 0, 0, 0 };
static char	vendor[32] = { "Generic" };
static char	brand[64] = { "Generic" };

static inline void  GetCPUInfo(  void )
{
    std::memset( vendor, 0, sizeof( vendor ) );
    std::memset( brand, 0, sizeof( brand ) );
    
    // Calling __cpuid with 0x0 as the function_id argument
	// gets the number of the highest valid function ID.
	__get_cpuid( CPU_VENDOR_BIT, &cpui[REG_EAX], &cpui[REG_EBX], &cpui[REG_ECX], &cpui[REG_EDX] );
	nIds = cpui[REG_EAX];

    // Calling __cpuid with 0x80000000 as the function_id argument
	// gets the number of the highest valid extended ID.
	__get_cpuid( CPU_BRAND_BIT, &cpui[REG_EAX], &cpui[REG_EBX], &cpui[REG_ECX], &cpui[REG_EDX] );
	nExIds = cpui[REG_EAX];

    // Capture vendor string
    std::memcpy( vendor + 0, &cpui[REG_EBX], sizeof( uint32_t ) );
    std::memcpy( vendor + 4, &cpui[REG_EDX], sizeof( uint32_t ) );
    std::memcpy( vendor + 8, &cpui[REG_ECX], sizeof( uint32_t ) );
    
    // Interpret CPU brand string if reported
	if ( nExIds >= 0x80000004 )
	{
		for ( uint32_t i = CPU_BRAND_BIT; i <= nExIds; ++i )
		{
			__get_cpuid( i, &cpui[REG_EAX], &cpui[REG_EBX], &cpui[REG_ECX], &cpui[REG_EDX] );

			if (i == 0x80000002)
				std::memcpy( brand, cpui, sizeof( cpui ) );
			else if (i == 0x80000003)
				std::memcpy( brand + 16, cpui, sizeof( cpui ) );
			else if (i == 0x80000004)
				std::memcpy( brand + 32, cpui, sizeof( cpui ) );
		}
	}
}

static inline bool HasCMOV( void )
{
    return cpui[REG_EDX] & BIT_CMOV;
}

static inline bool Has3DNow( void )
{
    // load bitset with flags for function 0x80000001
	if ( nExIds >= 0x80000001 )
	{
		__get_cpuid( 0x80000001, &cpui[REG_EAX], &cpui[REG_EBX], &cpui[REG_ECX], &cpui[REG_EDX] );

		// check for 3DNow!
		if ( cpui[REG_EDX] & BIT_3DNOW )
			return true;
	}

    return false;
}

static inline bool HasDAZ( void )
{
    // check for Denormals-Are-Zero mode
	uint32_t dwMask = _MM_GET_DENORMALS_ZERO_MODE();
	if ((dwMask & (1 << 6)) == (1 << 6))
		return true;

    return false;
}

/*
================
Sys_GetSystemRam

	returns amount of physical memory in MB
================
*/
int Sys_GetSystemRam( void )
{
	// SDL get the memory amount based in Mebibyte ( base 2 )
	return SDL_GetSystemRAM();
}


/*
========================
Sys_CPUCount

numLogicalCPUCores	- the number of logical CPU per core
numPhysicalCPUCores	- the total number of cores per package
numCPUPackages		- the total number of packages (physical processors)
========================
*/
// RB begin
void Sys_CPUCount( int& numLogicalCPUCores, int& numPhysicalCPUCores, int& numCPUPackages )
{
	numPhysicalCPUCores = SDL_GetNumLogicalCPUCores();
	numLogicalCPUCores = 1;
	numCPUPackages = 1;

	common->Printf( "/proc/cpuinfo CPU processors: %d\n", numPhysicalCPUCores );
	common->Printf( "/proc/cpuinfo CPU logical cores: %d\n", numLogicalCPUCores );
}

/*
================
Sys_GetProcessorId
================
*/
cpuid_t Sys_GetProcessorId( void )
{
    static uint32_t cpuid = CPUID_NONE;
 
    if ( cpuid != CPUID_NONE )
        return static_cast<cpuid_t>( cpuid );

    // check for an AMD
	if ( std::strcmp( vendor, "AuthenticAMD" ) == 0)
		cpuid = CPUID_AMD;
	// check for an Intel
	else if ( std::strcmp( vendor, "GenuineIntel" ) == 0)
		cpuid = CPUID_INTEL;
	else
		cpuid = CPUID_GENERIC;
    
    if( SDL_HasMMX() )
		cpuid |= CPUID_MMX;
	
    // REMOVED IN SDL3
    // check for 3DNow!
	if( Has3DNow() )
		cpuid |= CPUID_3DNOW;

    // check for Streaming SIMD Extensions
	if( SDL_HasSSE() )
		cpuid |= CPUID_SSE | CPUID_FTZ;
	
    // check for Streaming SIMD Extensions 2
	if( SDL_HasSSE2() )
		cpuid |= CPUID_SSE2;
	
    // check for Streaming SIMD Extensions 3 aka Prescott's New Instructions
	if( SDL_HasSSE3() )
		cpuid |= CPUID_SSE3;
	
    // check for Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
	if( HasCMOV() )
		cpuid |= CPUID_CMOV;
	
    if( HasDAZ() )
        cpuid |= CPUID_DAZ;
    
	return static_cast<cpuid_t>( cpuid );
}


/*
===============================================================================

	FPU

===============================================================================
*/

/*
===============
Sys_FPU_PrintStateFlags
===============
*/
int Sys_FPU_PrintStateFlags( char* ptr, int ctrl, int stat, int tags, int inof, int inse, int opof, int opse )
{
    return 0;
}

/*
===============
Sys_FPU_StackIsEmpty
===============
*/
bool Sys_FPU_StackIsEmpty( void )
{
    // Modern CPUs (SSE/AVX) do not have an x87 stack, so we consider it empty.
    // If you use legacy x87 code, implement x86-only check with fstsw.
    return true;
}

/*
===============
Sys_FPU_ClearStack
===============
*/
void Sys_FPU_ClearStack( void )
{
    // Portable: clear floating exceptions and do nothing to x87 state.
    std::feclearexcept(FE_ALL_EXCEPT);
    // No x87 stack to pop in modern SSE/AVX code.
}

/*
===============
Sys_FPU_GetState

  gets the FPU state without changing the state
===============
*/
const char* Sys_FPU_GetState()
{
	return "LEGADO";
}

/*
===============
Sys_FPU_EnableExceptions
===============
*/
void Sys_FPU_EnableExceptions( int exceptions )
{
#if __COMPILER_MSVC__
    unsigned int mask = 0;
    if (exceptions & 1) mask |= _EM_INVALID;
    if (exceptions & 2) mask |= _EM_ZERODIVIDE;
    if (exceptions & 4) mask |= _EM_OVERFLOW;
    if (exceptions & 8) mask |= _EM_UNDERFLOW;
    if (exceptions & 16) mask |= _EM_INEXACT;

    // 0 = ativa exceção, invertendo os bits
    _controlfp(~mask, _MCW_EM);

#elif __COMPILER_GCC__ || __COMPILER_CLANG__
    int mask = 0;
    if (exceptions & 1) mask |= FE_INVALID;
    if (exceptions & 2) mask |= FE_DIVBYZERO;
    if (exceptions & 4) mask |= FE_OVERFLOW;
    if (exceptions & 8) mask |= FE_UNDERFLOW;
    if (exceptions & 16) mask |= FE_INEXACT;

    feenableexcept(mask);
#else
    (void)exceptions; // stub para outras arquiteturas
#endif
}

/*
===============
Sys_FPU_SetPrecision
===============
*/
void Sys_FPU_SetPrecision( int precision )
{
    // NO-OP on modern CPUs. Precision control (x87 PC bits) is legacy and not portable.
    // If you *must* change x87 precision, implement platform-specific code (asm/_controlfp).
    // We intentionally do nothing to avoid x87 usage.
    // Keep function so legacy calls compile, but it's effectively ignored.
}

/*
================
Sys_FPU_SetRounding
================
*/
void Sys_FPU_SetRounding( int rounding )
{
    // rounding = 0,1,2,3 -> FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO
    static const int roundingModes[4] = 
    {
        FE_TONEAREST,
        FE_DOWNWARD,
        FE_UPWARD,
        FE_TOWARDZERO
    };

    const int mode = roundingModes[rounding & 3];

#if _ARCH_x86_32_ || _ARCH_x86_64_ // has X87
    // --- x87 FPU ---
    std::fesetround( mode );
#endif

    // --- SSE/AVX ---
    uint32_t mxcsr = _mm_getcsr();
    mxcsr &= ~(3 << 13);            // limpa bits de arredondamento (13 e 14)
    mxcsr |= ((rounding & 3) << 13);
    _mm_setcsr(mxcsr);
}

/*
================
Sys_FPU_SetDAZ
================
*/
void Sys_FPU_SetDAZ( bool enable )
{
	int mode = _MM_GET_DENORMALS_ZERO_MODE();
	if (enable && mode != _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );
	else if (!enable && mode == _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_OFF );
}

/*
================
Sys_FPU_SetFTZ
================
*/
void Sys_FPU_SetFTZ( bool enable )
{
	int mode = _MM_GET_FLUSH_ZERO_MODE();
	if (enable && mode != _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	else if (!enable && mode == _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_OFF );
}