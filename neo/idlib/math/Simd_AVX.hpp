
#ifndef __SIMD_AVX__
#define __SIMD_AVX__

/*
===============================================================================

	AVX implementation of idSIMDProcessor

===============================================================================
*/
class idSIMD_AVX : public idSIMD_SSE41
{
public:
	virtual const char* VPCALL GetName( void ) const;
    virtual void VPCALL Memcpy( void* dst,          const void* src,	const size_t count );
	virtual void VPCALL Memset( void* dst,	        const int val,		const size_t count );
    virtual void VPCALL	AtemporalCopy( void* dst,	const void* src,    const size_t bytes );
};

#endif //__SIMD_SSE41__