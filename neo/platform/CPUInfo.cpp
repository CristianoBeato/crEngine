#include "precompiled.h"
#include "platform.hpp"
#include "CPUInfo.hpp"
#include <cfenv>
#include <SDL3/SDL_cpuinfo.h>

/*
================
crCPUInfoLocal::FPUEnableExceptions
================
*/
void crCPUInfo::FPUEnableExceptions(const FPUExceptions_t in_exceptions)
{
    int exceptions = (int)in_exceptions; 
#if __COMPILER_MSVC__ || __COMPILER_MINGW__
    unsigned int mask = 0;
    if (exceptions & 1) 
        mask |= _EM_INVALID;
    if (exceptions & 2) 
        mask |= _EM_ZERODIVIDE;
    if (exceptions & 4) 
        mask |= _EM_OVERFLOW;
    if (exceptions & 8) 
        mask |= _EM_UNDERFLOW;
    if (exceptions & 16) 
        mask |= _EM_INEXACT;

    // 0 = ativa exceção, invertendo os bits
    _controlfp(~mask, _MCW_EM);

#elif __COMPILER_GCC__ || __COMPILER_CLANG__
    int mask = 0;
    if (exceptions & 1) 
        mask |= FE_INVALID;
    if (exceptions & 2) 
        mask |= FE_DIVBYZERO;
    if (exceptions & 4) 
        mask |= FE_OVERFLOW;
    if (exceptions & 8) 
        mask |= FE_UNDERFLOW;
    if (exceptions & 16) 
        mask |= FE_INEXACT;
    feenableexcept(mask);
#else
    (void)exceptions; // stub para outras arquiteturas
#endif
}

/*
================
crCPUInfoLocal::FPUSetRounding
================
*/
void crCPUInfo::FPUSetRounding( const FPURounding_t in_rounding )
{
    int rounding = (int)in_rounding;
    // rounding = 0,1,2,3 -> FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO
    static const int roundingModes[4] = 
    {
        FE_TONEAREST,
        FE_DOWNWARD,
        FE_UPWARD,
        FE_TOWARDZERO
    };

    const int mode = roundingModes[rounding & 3];

#if _ARCH_x86_64_ || _ARCH_x86_32_ // has X87
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
crCPUInfoLocal::FPUSetFTZ
================
*/
void crCPUInfo::FPUSetFTZ( const bool in_enable )
{
    // check current status
	int mode = _MM_GET_FLUSH_ZERO_MODE();
	if ( in_enable && mode != _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	else if (!in_enable && mode == _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_OFF );
}

/*
================
crCPUInfoLocal::FPUSetDAZ
================
*/
void crCPUInfo::FPUSetDAZ( const bool in_enable )
{
	int mode = _MM_GET_DENORMALS_ZERO_MODE();
	if ( in_enable && mode != _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );
	else if (! in_enable && mode == _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_OFF );
}

/*
=====================
crCPUInfo::GetSystemRam
=====================
*/
size_t crCPUInfo::GetSystemRam( void ) const
{
    // SDL get the memory amount based in Mebibyte ( base 2 )
	static size_t size = 0;
	if( size == 0 )
		size = SDL_GetSystemRAM();

    return size;
}
