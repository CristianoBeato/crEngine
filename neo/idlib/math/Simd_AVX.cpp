
#include "Simd_AVX.hpp"

void VPCALL	idSIMD_AVX::AtemporalCopy( void* dst,	const void* src,    const size_t bytes )
{
    idassert(reinterpret_cast<uintptr_t>( dst ) % 32 == 0);
    idassert(reinterpret_cast<uintptr_t>( src ) % 32 == 0);

    auto* __restrict d = static_cast<__m256i*>( dst );
    const auto* __restrict s = static_cast<const __m256i*>( src );
    
    // Processa 64 bytes por iteração (uma cache line completa da CPU)
    size_t num_blocks_256 = bytes / sizeof(__m256i);
    for ( size_t i = 0; i < num_blocks_256; i += 2 ) 
    {
        __m256i r0 = _mm256_loadu_si256(&s[i]);
        __m256i r1 = _mm256_loadu_si256(&s[i + 1]);

        _mm256_stream_si256(&d[i],     r0);
        _mm256_stream_si256(&d[i + 1], r1);
    }

    _mm_sfence(); 
}