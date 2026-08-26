
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

class crConsole
{
public:
    static crConsole*   Get( void );

    crConsole( void ) {};
    virtual ~crConsole( void ) {};
	virtual void			Shutdown( void ) = 0;
	virtual void			ShowConsole( const int in_visLevel, const bool in_quitOnClose ) = 0;
	virtual void            Printf( const char *fmt, ... ) = 0;
    virtual void            VPrintf( const char *fmt, va_list arg ) = 0;
	virtual void            DebugPrintf( const char *fmt, ... ) = 0;
    virtual void            DebugVPrintf( const char *fmt, va_list arg ) = 0;
	virtual void			Error( const char* fmt, ... ) = 0;
    virtual void            VError( const char *fmt, va_list arg ) = 0; 
    virtual const char *    GetCmdLine( void ) = 0;
};

class crDisplay
{
public:
    virtual const char* Name( void ) const = 0;
    virtual const vidMode_t* Modes( uint32_t *in_count ) const = 0;
};

typedef class crVideo* crVideop;
class crVideo
{
public:
	static crVideop				Get( void );

	/// @brief Initialize video management system, list displays and video modes 
	/// @param in_flags 
	/// @return true on success, false on error 
	virtual bool    			StartUp( const uint32_t in_flags ) = 0;

	/// @brief Release video system
	/// @param  
	virtual void    			ShutDown( void ) = 0;

	/// @brief Retrieve native window handler 
	/// @return pointer to handle object
	virtual void*				WindowHandler( void ) = 0;
	
	/// @brief Grab mouse and keyboard to window 
	/// @param in_flags 
	virtual void				GrabInput( const uint32_t in_flags ) = 0;

	/// @brief Set window to a 
	/// @param in_mode 
	/// @param in_fullScreen 
	/// @return 
	virtual bool				SetMode( const vidMode_t in_mode, const videoMode_t in_fullScreen ) = 0;

	/// @brief Configure video gama ( TODO: Move to renderer )
	/// @param red gama curve
	/// @param green gama curve
	/// @param blue gama cuver
	virtual void				SetGamma( uint16_t red[256], uint16_t green[256], uint16_t blue[256] ) = 0;

	/// @brief Show or hide window 
	/// @param show on true show window if hide, on false hiden window 
	virtual void				ShowWindow( bool show ) = 0;

	/// @brief Enable or disable text input event, it enable window to record
	/// keyboard events as text input, utilized by the console.
	/// @param in_enable set text input moode enable
	virtual void				TextInput( const bool in_enable ) = 0;	

	/// @brief Check if window is visible
	/// @return true if window is not hiden 
	virtual bool				IsWindowVisible( void ) const = 0;

	/// @brief 
	/// @return 
	virtual crDisplay* const* 	Displays( uint32_t* in_count ) const = 0;
};

/// Render API Context Management

typedef class crRenderDevice* crRenderDevicep;
class crRenderDevice
{
public:
	struct properties_t
	{
		bool		BCnTextureCompression = false;
		bool		ETC2TextureCompression = false;
		bool		asotropicFiltering = false;
		bool		sRGBFramebufferAvailable = false;
		bool		textureFloatAvailable = false;
		bool		depthBoundsTestAvailable = false;
		bool		timerQueryAvailable = false;
		bool		occlusionQueryAvailable = false;

		uint16_t 	deviceID = 0;
		uint16_t 	vendorID = 0;
		uint32_t 	driverVersion = 0;
		
		uint32_t	maxTextureSize = 0;
		uint32_t	maxSampleCount = 0;
		uint32_t 	shaderStorageAlignment = 0;
		float		maxAnisotropicFiltering = 0.0f;
		float		maxTextureLODBias = 0.0f;
		float		timestampPeriod = 0.0f;

		/// buffes aligments
		size_t 		uniformBuffersAlignment = 0;
		size_t 		storageBuffersAlignment = 0;
		size_t		texelBufferOffsetAlignment = 0;
	};

	virtual bool				Create(  const char** in_layers, const uint32_t in_numLayers, const char** in_enabledExtensions, const uint32_t in_numExtensions ) = 0;
	virtual void				Destroy( void ) = 0;
	virtual const char*			Name( void ) const = 0;
	virtual const properties_t	Properties( void ) const = 0;
	virtual const int32_t		Score( void ) const = 0;
	virtual bool				ReloadCache( void ) = 0;
	virtual const bool      	ExtensionAvailable( const char* in_ext ) const = 0;
};

/// Render API Context Management
class crRenderAPI
{
public:
	static crRenderAPI* Get( void );
	virtual bool				StartUp( void ) = 0;
	virtual void				ShutDown( void ) = 0;
	virtual uint32_t			GetDevices( crRenderDevicep* m_deviceArray ) = 0;
};
// BEATO End

// mouse input polling
inline constexpr int MAX_MOUSE_EVENTS = 256;
inline constexpr int MAX_JOYSTICKS = 4; // Limit for Most consoles is 4 Controllers 
class crInputSystem
{
public:
	static crInputSystem*	Get( void );
	crInputSystem( void ) {};
	~crInputSystem( void ) {};

	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	virtual void					Init( void ) = 0;
	virtual void					Shutdown( void ) = 0;
	// event generation
	virtual void					GenerateEvents( void ) = 0;
	virtual sysEvent_t				GetEvent( void ) = 0;
	virtual void					ClearEvents( void ) = 0;
	
	virtual const unsigned char*	GetScanTable( void ) = 0;
	
	// keyboard input polling
	virtual int						PollKeyboardInputEvents( void ) = 0;
	virtual int						ReturnKeyboardInputEvent( const int n, int& ch, bool& state ) = 0;
	virtual void					EndKeyboardInputEvents( void ) = 0;
	
	// mouse polling
	virtual int						PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] ) = 0;
	virtual sysEvent_t				GenerateMouseButtonEvent( const int button, const bool down ) = 0;
	virtual sysEvent_t 				GenerateMouseMoveEvent( const int32_t deltax, const int32_t deltay ) = 0;

	// joystick input polling
	virtual uint32_t				GamepadCount( void ) = 0;
	virtual void					SetRumble( const int device, uint16_t in_low, uint16_t in_hi ) = 0;
	virtual int						PollJoystickInputEvents( const int in_deviceNum ) = 0;
	virtual bool					ReturnJoystickInputEvent( const int n, int& in_action, int& in_value ) = 0;
	virtual void					EndJoystickInputEvents( void ) = 0;

	// when the console is down, or the game is about to perform a lengthy
	// operation like map loading, the system can release the mouse cursor
	// when in windowed mode
	virtual void					GrabMouseCursor( const bool in_grabIt ) = 0;	
};

typedef struct sysMemoryStats_e
{
	size_t memoryLoad;
	size_t totalPhysical;
	size_t availPhysical;
	size_t totalPageFile;
	size_t availPageFile;
	size_t totalVirtual;
	size_t availVirtual;
    size_t availExtendedVirtual;
} sysMemoryStats_t;

class crCPUInfo
{
public: 
	static crCPUInfo* Get( void );
	crCPUInfo( void ) {};
	~crCPUInfo( void ) {};

	enum cpuid_t
	{
		CPUID_NONE							= 0x00000,
		CPUID_UNSUPPORTED					= ( 1 << 0 ),	// unsupported (386/486)
		CPUID_GENERIC						= ( 1 << 1 ),	// unrecognized processor
		CPUID_INTEL							= ( 1 << 2 ),	// Intel
		CPUID_AMD							= ( 1 << 3 ),	// AMD
		CPUID_ARM							= ( 1 << 4 ),	// ARM cpu
		CPUID_MMX							= ( 1 << 5 ),	// Multi Media Extensions
		CPUID_3DNOW							= ( 1 << 6 ),	// 3DNow!
		CPUID_SSE							= ( 1 << 7 ),	// Streaming SIMD Extensions
		CPUID_SSE2							= ( 1 << 8 ),	// Streaming SIMD Extensions 2
		CPUID_SSE3							= ( 1 << 9 ),	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
		CPUID_SSSE3							= ( 1 << 10 ),	//
		CPUID_SSE41							= ( 1 << 11 ),	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
		CPUID_SSE42							= ( 1 << 12 ),	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
		CPUID_ALTIVEC						= ( 1 << 13),	// AltiVec
		CPUID_CMOV							= ( 1 << 15 ),	// Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
		CPUID_FTZ							= ( 1 << 16 ),	// Flush-To-Zero mode (denormal results are flushed to zero)
		CPUID_DAZ							= ( 1 << 17 ),	// Denormals-Are-Zero mode (denormal source operands are set to zero)
	};

	enum FPUExceptions_t
	{
		FPU_EXCEPTION_INVALID_OPERATION		= 1,
		FPU_EXCEPTION_DENORMALIZED_OPERAND	= 2,
		FPU_EXCEPTION_DIVIDE_BY_ZERO		= 4,
		FPU_EXCEPTION_NUMERIC_OVERFLOW		= 8,
		FPU_EXCEPTION_NUMERIC_UNDERFLOW		= 16,
		FPU_EXCEPTION_INEXACT_RESULT		= 32
	};

	enum FPUPrecision_t
	{
		FPU_PRECISION_SINGLE				= 0,
		FPU_PRECISION_DOUBLE				= 1,
		FPU_PRECISION_DOUBLE_EXTENDED		= 2
	};

	enum FPURounding_t
	{
		FPU_ROUNDING_TO_NEAREST				= 0,
		FPU_ROUNDING_DOWN					= 1,
		FPU_ROUNDING_UP						= 2,
		FPU_ROUNDING_TO_ZERO				= 3
	};

	virtual void	Init( void ) = 0;

	/// @brief returns a selection of the CPUID_* flags
	inline uint32_t			GetProcessorId( void ) const { return m_cpuIDFlags; };
	inline const char*		GetProcessorString( void ) const { return &m_ProcessorName[0]; };
	inline const uint32_t	GetProcessorThreads( void ) const { return m_cpuThreads; }
 
	// enables the given FPU exceptions
	void		FPUEnableExceptions( const FPUExceptions_t in_exceptions );

	// sets the FPU rounding mode
	void		FPUSetRounding( const FPURounding_t in_rounding );

	// sets Flush-To-Zero mode (only available when CPUID_FTZ is set)
	void		FPUSetFTZ( const bool in_enable );

	/// sets Denormals-Are-Zero mode (only available when CPUID_DAZ is set)
	void		FPUSetDAZ( const bool in_enable );

	/// @brief returns amount of physical memory in MB
	size_t		GetSystemRam( void ) const;

protected:
	char		m_ProcessorName[256];
	uint32_t	m_cpuIDFlags;
	uint32_t	m_cpuThreads;
};

#define ID_LANG_ENGLISH		"english"
#define ID_LANG_FRENCH		"french"
#define ID_LANG_ITALIAN		"italian"
#define ID_LANG_GERMAN		"german"
#define ID_LANG_SPANISH		"spanish"
#define ID_LANG_JAPANESE	"japanese"

class crPlatform
{
public:
    static crPlatform*  Get( void );

    crPlatform( void ) {};
    virtual ~crPlatform( void ) {};

    virtual void    	Init( void ) = 0;
    virtual void    	Shutdown( void ) = 0;
    virtual void    	Quit( void ) = 0;
    virtual void    	Exit( const int code ) = 0;
    virtual bool    	AlreadyRunning( void ) = 0;
    virtual bool    	CreateInstanceLock( void ) = 0;

    /// @brief lock memory caching restriction
    virtual bool    	LockMemory( void* ptr, const size_t bytes ) = 0;
    
    /// @brief unlock memory caching restriction
    virtual bool    	UnlockMemory( void* ptr, const size_t bytes ) = 0;

	virtual const char*	GetCmdLine( void );
    virtual void    	ReLaunch( void * data, const size_t dataSize ) = 0;
    virtual void    	StartProcess( const char *exePath, const bool doexit ) = 0;
    virtual void    	OpenURL( const char *url, const bool doexit ) = 0;

    /// @brief returns current OS mem info, all values are in kB except the memoryload
    virtual void    	GetCurrentMemoryStatus( sysMemoryStats_t& stats ) = 0;

    /// @brief get the OS mem info, from engine boot
    virtual void    	GetExeLaunchMemoryStatus( sysMemoryStats_t &stats ) = 0;

    // know early if we are performing a fatal error shutdown so the error message doesn't get lost
    virtual void    	SetFatalError( const char* error ) = 0;

    /// 
    virtual const char* GetCurrentUser( void ) = 0;

	/// @brief return number of supported languages 
	const uint32_t		NumLangs( void ) const;

	/// @brief get language name by index
	const char*			Language( const uint32_t in_idx ) const;
	void				SetLanguageFromSystem( void );
	const char* 		DefaultLanguage( void ) const;


    ///====================================================================
    /// SDL3 Portable calls
    ///====================================================================

    /// @brief Load a local shared library.
    void* SharedLibLoad( const char *in_libraryName ) const;

    /// @brief Retrieve a function pointer from a shared library.
    void *SharedLibProcAddress( void* in_handle, const char *in_procName ) const;

    /// @brief Unload a shared lib handle.
    void SharedLibUnload( void* in_handle ) const;

    /// @brief Retrieve a text string from the system clipboard.
    /// @return UTF-8 string, must be free whit Mem_Free.
    const char*         GetClipboardData( void ) const;

    /// @brief Store a string in the system clipboard.
    /// @param in_string string to bi copied to clipboard.
    void                SetClipboardData( const char* in_string ) const;

    /// @brief current thread waits a specified number of milliseconds before returning.
    /// @param  in_milliseconds amount of milliseconds to wait
    const bool          Sleep( const uint32_t in_milliseconds ) const;

    /// @brief Get the number of milliseconds that have elapsed since the engine initialization.
    const uint32_t      Milliseconds( void ) const;

    /// @brief Get the number of microseconds that have elapsed since the engine initialization.
    const uint64_t      Microseconds( void ) const;
};

#endif //__PLATFORM_H__