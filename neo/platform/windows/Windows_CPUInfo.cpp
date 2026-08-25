
#include "idlib/precompiled.h"
#include "../Platform.hpp"
#include "Windows_CPUInfo.hpp"

#include <SDL3/SDL_cpuinfo.h>
#include <windows.h>

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

    SDL_GetNumLogicalCPUCores();
    SDL_GetCPUCacheLineSize();
    SDL_HasAltiVec();
    SDL_HasMMX();
    SDL_HasSSE();
    SDL_HasSSE2();
    SDL_HasSSE3();
    SDL_HasSSE41();
    SDL_HasSSE42();
    SDL_HasAVX();
    SDL_HasAVX2();
    SDL_HasAVX512F();
    SDL_HasARMSIMD();
    SDL_HasNEON();

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
        m_ProcessorName = idStr( "Unkow Processor" );
        return;
    }

     /// read string value from "ProcessorNameString"
    LONG readStatus = RegQueryValueExA( hKey, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize );

    /// release the key 
    RegCloseKey( hKey );

    if (readStatus != ERROR_SUCCESS) 
    {
        idLib::Warning( "Failed to read Win Register \"ProcessorNameString\"" );
        m_ProcessorName = idStr( "Unkow Processor" );
        return;
    }

    /// Now we check if are a AMD or Intel
    if ( std::strstr(buffer, "AMD") != nullptr || std::strstr(buffer, "amd") != nullptr )
        m_cpuIDFlags |= CPUID_AMD;

    if ( std::strstr(buffer, "Intel") != nullptr || std::strstr(buffer, "intel") != nullptr )
        m_cpuIDFlags |= CPUID_INTEL;

    if ( std::strstr(buffer, "ARM") != nullptr || std::strstr(buffer, "arm") != nullptr )
        m_cpuIDFlags |= CPUID_ARM;

    m_ProcessorName = buffer;
}
