
#include "Simd_SSE41.hpp"

void VPCALL idSIMD_SSE41::Memcpy( void* dst,  const void* src, const size_t count )
{
}

void VPCALL idSIMD_SSE41::Memset( void* dst,	const int val, const size_t count )
{
}

void VPCALL idSIMD_SSE41::AtemporalCopy( void *dst, const void *src, const size_t bytes )
{
    idassert(reinterpret_cast<uintptr_t>( dst ) % 16 == 0);
    idassert(reinterpret_cast<uintptr_t>( src ) % 16 == 0);


    auto* __restrict d = static_cast<__m128i*>( dst );
    const auto* __restrict s = static_cast<const __m128i*>( src );

    // Processa 64 bytes por iteração (uma cache line completa da CPU)
    size_t num_blocks_128 = bytes / sizeof(__m128i);
    for (size_t i = 0; i < num_blocks_128; i += 4) 
    {
        __m128i r0 = _mm_loadu_si128(&s[i]);
        __m128i r1 = _mm_loadu_si128(&s[i + 1]);
        __m128i r2 = _mm_loadu_si128(&s[i + 2]);
        __m128i r3 = _mm_loadu_si128(&s[i + 3]);

        // Streaming stores bypassam o cache da CPU escrevendo direto no buffer WC
        _mm_stream_si128(&d[i],     r0);
        _mm_stream_si128(&d[i + 1], r1);
        _mm_stream_si128(&d[i + 2], r2);
        _mm_stream_si128(&d[i + 3], r3);
    }

    // insert a fence to signal the end of the copy operations 
    _mm_sfence(); 
}
