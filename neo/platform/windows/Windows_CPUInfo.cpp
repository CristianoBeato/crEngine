
#include "idlib/precompiled.h"
#include "../Platform.hpp"
#include "Windows_CPUInfo.hpp"

#include <SDL3/SDL_cpuinfo.h>
#include <windows.h>
#include <intrin.h>

constexpr int _REG_EAX = 0;
constexpr int _REG_EBX = 1;
constexpr int _REG_ECX = 2;
constexpr int _REG_EDX = 3;

static bool HasCMOV( void )
{
    int cpuInfo[4]{ 0, 0, 0, 0 };
    
    // get CPU feature bits
    __cpuid(cpuInfo, 1 );

    // bit 15 of EDX denotes CMOV existence
	if ( cpuInfo[_REG_EDX] & ( 1 << 15 ) )
        return true;

    return false;
}

static bool HasDAZ( void )
{
    int cpuInfo[4]{ 0, 0, 0, 0 };
    
    // Read the MXCSR Mask
    unsigned int mxcsr = _mm_getcsr();

    return ( ( mxcsr & ( 1 << 6 ) ) == ( 1 << 6 ) );	// Return if the DAZ bit is set
}

crCPUInfo* crCPUInfo::Get( void )
{
    static crWindowsCPUInfo gWindowsCPUInfo = crWindowsCPUInfo();
    return &gWindowsCPUInfo;
}

crWindowsCPUInfo::crWindowsCPUInfo( void )
{
}

crWindowsCPUInfo::~crWindowsCPUInfo( void )
{
}

void crWindowsCPUInfo::Init(void)
{
    uint32_t CPUArchitecture = 0;

    /// Retrieve the proc name
    GetProcessorName();

    m_cpuThreads = SDL_GetNumLogicalCPUCores();
    m_cpuCacheLines = SDL_GetCPUCacheLineSize();
    
    if( SDL_HasAltiVec() )
        CPUArchitecture |= CPUID_ALTIVEC;

    if( SDL_HasMMX() )
        CPUArchitecture |= CPUID_MMX;

    if( SDL_HasSSE() )
        CPUArchitecture |= CPUID_SSE;

    if( SDL_HasSSE2() )
        CPUArchitecture |= CPUID_SSE2;

    if( SDL_HasSSE3() )
        CPUArchitecture |= CPUID_SSE3;

    if( SDL_HasSSE41() )
        CPUArchitecture |= CPUID_SSE41;

    if( SDL_HasSSE42() )
        CPUArchitecture |= CPUID_SSE41;

    if( SDL_HasAVX() )
        CPUArchitecture |= CPUID_AVX;
    if( SDL_HasAVX2() )
        CPUArchitecture |= CPUID_AVX2;

    if( SDL_HasAVX512F() )
        CPUArchitecture |= CPUID_AVX512;

    if( HasCMOV() )
        CPUArchitecture |= CPUID_CMOV;

    if( HasDAZ() )
        CPUArchitecture |= CPUID_DAZ;

    m_cpuIDFlags |= CPUArchitecture;
}

void crWindowsCPUInfo::GetProcessorName(void)
{
    char buffer[256];
    DWORD bufferSize = sizeof(buffer);
    HKEY hKey;

    /// try opens the registry key containing CPU information
    LONG openStatus = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey );

    if (openStatus != ERROR_SUCCESS) 
    {
        idLib::Warning( "Failed to open Win Register HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0" );
        idStr::CopyString( m_ProcessorName, "Unkow Processor", 256 );
        return;
    }

     /// read string value from "ProcessorNameString"
    LONG readStatus = RegQueryValueExA( hKey, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize );

    /// release the key 
    RegCloseKey( hKey );

    if (readStatus != ERROR_SUCCESS) 
    {
        idLib::Warning( "Failed to read Win Register \"ProcessorNameString\"" );
        idStr::CopyString( m_ProcessorName, "Unkow Processor", 256 );
        return;
    }

    /// Now we check if are a AMD or Intel
    if ( std::strstr(buffer, "AMD") != nullptr || std::strstr(buffer, "amd") != nullptr )
        m_cpuIDFlags |= CPUID_AMD;

    if ( std::strstr(buffer, "Intel") != nullptr || std::strstr(buffer, "intel") != nullptr )
        m_cpuIDFlags |= CPUID_INTEL;

    if ( std::strstr(buffer, "ARM") != nullptr || std::strstr(buffer, "arm") != nullptr )
        m_cpuIDFlags |= CPUID_ARM;

    SDL_snprintf( m_ProcessorName, 256, buffer );
}
