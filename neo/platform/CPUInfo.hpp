
#ifndef __PLATFORM_HPP__
#define __PLATFORM_HPP__

class crCPUInfoLocal : public crCPUInfo
{
public:
    crCPUInfoLocal( void );
    ~crCPUInfoLocal( void );

    /// @brief returns a selection of the CPUID_* flags
	cpuid_t			GetProcessorId( void );
	const char* 	GetProcessorString( void );

	/// @brief enables the given FPU exceptions
	virtual void	FPUEnableExceptions( const FPUExceptions_t in_exceptions );

	/// @brief sets the FPU precision
	virtual void	FPUSetPrecision( const FPUPrecision_t in_precision );

	/// @brief sets the FPU rounding mode
	virtual void	FPUSetRounding( const FPURounding_t in_rounding );

	/// @brief sets Flush-To-Zero mode (only available when CPUID_FTZ is set)
	virtual void	FPUSetFTZ( const bool in_enable );

	/// @brief sets Denormals-Are-Zero mode (only available when CPUID_DAZ is set)
	virtual void	FPUSetDAZ( const bool in_enable );

	/// @brief returns amount of system ram
	virtual size_t	GetSystemRam( void ) const;

private:

};



#endif //!__PLATFORM_HPP__