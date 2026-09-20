
#include "idlib/precompiled.h"
#include "../Platform.hpp"
#include "Linux_CPUInfo.hpp"

#include <SDL3/SDL_cpuinfo.h>

crCPUInfo* crCPUInfo::Get()
{
    static crLinuxCPUInfo gLinuxCPUInfo = crLinuxCPUInfo();
    return &gLinuxCPUInfo;
}

crLinuxCPUInfo::crLinuxCPUInfo( void ) : crCPUInfo()
{
}

crLinuxCPUInfo::~crLinuxCPUInfo( void )
{
}

void crLinuxCPUInfo::Init(void)
{
    uint32_t CPUArchitecture = 0;

    /// Retrieve the proc name
    GetProcessorName();

    m_cpuThreads = SDL_GetNumLogicalCPUCores();
    m_cpuCacheLines = SDL_GetCPUCacheLineSize();

    if( SDL_HasMMX() ) CPUArchitecture |= CPUID_MMX;    
    if( SDL_HasSSE() ) CPUArchitecture |= CPUID_SSE;
    if( SDL_HasSSE2() ) CPUArchitecture |= CPUID_SSE2;
    if( SDL_HasSSE3() ) CPUArchitecture |= CPUID_SSE3;
    if( SDL_HasSSE41()) CPUArchitecture |= CPUID_SSE41;
    if( SDL_HasSSE42()) CPUArchitecture |= CPUID_SSE2;
    if( SDL_HasAVX()) CPUArchitecture |= CPUID_AVX;
    if( SDL_HasAVX2()) CPUArchitecture |= CPUID_AVX2;
    if( SDL_HasAVX512F()) CPUArchitecture |= CPUID_AVX512;
    if( SDL_HasAltiVec()) CPUArchitecture |= CPUID_ALTIVEC;
    //if( SDL_HasARMSIMD()) CPUArchitecture |=
    //if( SDL_HasNEON()) CPUArchitecture |=
    
    m_cpuIDFlags |= CPUArchitecture;
}   

void crLinuxCPUInfo::GetProcessorName(void)
{
    bool    foundModel = false;
    int     foundVendor = 0;
    size_t  flegenth = 0;
    char    modelName[256];
    char    line[512];
    
    /// trye read CPU description file 
    FILE* cpuinfo_file = fopen64( "/proc/cpuinfo", "r" );
    if( !cpuinfo_file )
    {
        // failed, 
        SDL_snprintf( m_ProcessorName, 256, "Unknow" );
        idLib::Error( "Faild to open \"/proc/cpuinfo\"");
        return;
    }

    // read file line by using fgets
    while ( std::fgets( line, sizeof(line), cpuinfo_file ) ) 
    {
        // Search for the processor brand
        if ( !foundModel && std::strstr(line, "model name") != nullptr ) 
        {
            char* colon = std::strchr(line, ':');
            if ( colon != nullptr ) 
            {
                // Advance the pointer to skip the ':' and the space following it
                char* nameStart = colon + 2; 
                
                // Remove the newline character '\n' that fgets includes at the end
                size_t len = std::strlen(nameStart);
                if (len > 0 && nameStart[len - 1] == '\n') 
                    nameStart[len - 1] = '\0';
                
                // Safely copies to the structure
                std::strncpy( modelName, nameStart, sizeof(modelName) - 1);
                modelName[sizeof( modelName) - 1] = '\0'; // Garante o null-terminator
                foundModel = 1;
            }
        }

        // Fallback for the name on ARM architectures (where the field is named "Processor")
        if (!foundModel && std::strstr(line, "Processor") != nullptr ) 
        {
            char* colon = std::strchr(line, ':');
            if (colon != nullptr) 
            {
                char* nameStart = colon + 2;
                size_t len = std::strlen(nameStart);
                if (len > 0 && nameStart[len - 1] == '\n') 
                    nameStart[len - 1] = '\0';

                std::strncpy( modelName, nameStart, sizeof( modelName ) - 1);
                modelName[sizeof(modelName) - 1] = '\0';
                foundModel = true;
            }
        }

        // Searches for the exact manufacturer (vendor_id)
        if (!foundVendor && std::strstr(line, "vendor_id") != nullptr ) 
        {
            if ( std::strstr(line, "GenuineIntel") != nullptr ) 
            {
                SDL_snprintf( m_VendorName, 64, "Intel" );
                m_cpuIDFlags |= CPUID_INTEL;
                foundVendor = 1;
            }
            else if ( std::strstr(line, "AuthenticAMD") != nullptr ) 
            {
                SDL_snprintf( m_VendorName, 64, "AMD" );
                m_cpuIDFlags |= CPUID_AMD;
                foundVendor = 1;
            }
        }

        // We found what we need, stop the loop
        if (foundModel && foundVendor)
            break;
    }

    //
    fclose( cpuinfo_file );

#if 0
    // Architecture fallback for non-classic x86 (Intel/AMD) systems
    if ( strcmp( m_VendorName, "Desconhecido") == 0 && foundModel ) 
    {

        if (std::strstr( modelName, "ARM") != NULL || strstr(info.modelName, "aarch64") != NULL) 
        {
            strcpy(info.vendor, "Apenas ARM");
        } 
        else if (strstr(info.modelName, "riscv") != NULL || strstr(info.modelName, "RISC-V") != NULL) 
        {
            strcpy(info.vendor, "Apenas RISC-V");
        }
    }
#endif
}
