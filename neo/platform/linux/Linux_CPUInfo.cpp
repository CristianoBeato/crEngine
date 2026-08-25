
#include "idlib/precompiled.h"
#include "../Platform.hpp"
#include "Linux_CPUInfo.hpp"


crLinuxCPUInfo::crLinuxCPUInfo( void )
{
}

crLinuxCPUInfo::~crLinuxCPUInfo( void )
{
}

void crLinuxCPUInfo::GetProcessorName(void)
{
    bool    foundModel = false;
    int     foundVendor = 0;
    size_t  flegenth = 0;
    char    modelName[256];
    char    line[512];
    
    
    FILE* cpuinfo_file = fopen64( "/proc/cpuinfo", "r" );
    if( !cpuinfo_file )
    {
        m_ProcessorName = idStr( "Unknow" );
        idLib::Error( "Faild to open \"/proc/cpuinfo\"");
        return;
    }

    // read file line by using fgets
    while (fgets(line, sizeof(line), cpuinfo_file)) 
    {
        
        // 1. Busca o modelo comercial do processador
        // Search for the processor brand
        if (!foundModel && strstr(line, "model name") != NULL ) 
        {
            char* colon = strchr(line, ':');
            if (colon != NULL) 
            {
                // Avança o ponteiro para pular o ':' e o espaço que vem depois dele
                char* nameStart = colon + 2; 
                
                // Remove a quebra de linha '\n' que o fgets traz no final
                size_t len = strlen(nameStart);
                if (len > 0 && nameStart[len - 1] == '\n') 
                    nameStart[len - 1] = '\0';
                
                // Copia com segurança para a estrutura
                std::strncpy( modelName, nameStart, sizeof(modelName) - 1);
                modelName[sizeof( info.modelName) - 1] = '\0'; // Garante o null-terminator
                foundModel = 1;
            }
        }

        // Fallback do nome para arquiteturas ARM (onde o campo se chama "Processor")
        if (!foundModel && std::strstr(line, "Processor") != NULL) 
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

        // 2. Busca a fabricante exata (vendor_id)
        if (!foundVendor && strstr(line, "vendor_id") != NULL) 
        {
            if ( std::strstr(line, "GenuineIntel") != NULL) 
            {
                std::strcpy( vendor, "Apenas Intel");
                foundVendor = 1;
            } 
            else if (std::strstr(line, "AuthenticAMD") != NULL) 
            {
                std::strcpy( vendor, "Apenas AMD" );
                foundVendor = 1;
            }
        }

        // Otimização: Se já achou tudo o que precisava na primeira CPU da lista, para o loop
        if (foundModel && foundVendor) {
            break;
        }
    }

    fclose( cpuinfo_file );

    // Fallback de arquitetura caso não seja um x86 clássico (Intel/AMD)
    if (strcmp(info.vendor, "Desconhecido") == 0 && foundModel) {

        if (strstr(info.modelName, "ARM") != NULL || strstr(info.modelName, "aarch64") != NULL) 
        {
            strcpy(info.vendor, "Apenas ARM");
        } 
        else if (strstr(info.modelName, "riscv") != NULL || strstr(info.modelName, "RISC-V") != NULL) 
        {
            strcpy(info.vendor, "Apenas RISC-V");
        }
    }
}
