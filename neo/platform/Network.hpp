
#ifndef __NETWORK_SYSTEM_HPP__
#define __NETWORK_SYSTEM_HPP__

/*
==============================================================

	Networking

==============================================================
*/

typedef enum
{
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef struct
{
	netadrtype_t	type;
	uint8_t	        ip[4];
	uint16_t	    port;
} netadr_t;

#define	PORT_ANY			-1

class crNetSoket
{
public:
	crNetSoket( void );
	~crNetSoket( void );
	virtual bool	Open( void ) = 0;
	virtual bool	Close( void ) = 0;
	virtual void	ReciveUDP( void* out_data, size_t &out_size, const size_t in_maxSize ) = 0;
	virtual void	SendUDP( const void in_data, const size_t in_size ) = 0;
};

/*
================================================
idUDP
================================================
*/
class idUDP
{
public:
	// this just zeros netSocket and port
	idUDP();
	virtual		~idUDP();
	
	// if the InitForPort fails, the idUDP.port field will remain 0
	bool		InitForPort( int portNumber );
	
	int			GetPort() const
	{
		return bound_to.port;
	}

	netadr_t	GetAdr() const
	{
		return bound_to;
	}

	uint32_t		GetUIntAdr() const
	{
		return ( bound_to.ip[0] | bound_to.ip[1] << 8 | bound_to.ip[2] << 16 | bound_to.ip[3] << 24 );
	}
	void		Close();
	
	bool		GetPacket( netadr_t& from, void* data, size_t& size, size_t maxSize );
	
	bool		GetPacketBlocking( netadr_t& from, void* data, size_t& size, size_t maxSize, int timeout );
								   
	void		SendPacket( const netadr_t to, const void* data, size_t size );
	
	void		SetSilent( bool silent )
	{
		this->silent = silent;
	}

	bool		GetSilent() const
	{
		return silent;
	}
	
	int			packetsRead;
	int			bytesRead;
	
	int			packetsWritten;
	int			bytesWritten;
	
	bool		IsOpen( void ) const
	{
		return netSocket > 0;
	}
	
private:
	netadr_t	bound_to;		// interface and port
	bool		silent;			// don't emit anything ( black hole )
#if USE_SDL3NET
	NET_DatagramSocket*	netSocket;
#else
	int					netSocket;		// OS specific socket
#endif 
};

// TODO: update to a class
struct crNetMessage
{
	crNetMessage( const size_t bufferSize = 32767, const bool compressed = true );
	~crNetMessage( void );

	void ResetReadOffset();

	bool ReadBytes(char * bytes, int numBytes);

	template<typename T>
	bool Read(T & output)
	{
		return ReadBytes((char*)&output, sizeof(T));
	}

	bool ReadString(char * buffer, int maxlength);

	bool WriteBytes(const char * bytes, int numBytes);

	template<typename T>
	bool Write(const T & output)
	{
		return WriteBytes((const char *)&output, sizeof(T));
	}

	bool WriteString(const char * output);

	bool ReadPacket(idUDP & socket, netadr_t & addrFrom);
	void SendPacket(idUDP & socket, const netadr_t & addr);
	
	inline const char * Data ( void ) const{ return mData; }
	inline size_t Size( void ) const { return mDataOffset; }


private:
	const bool		mCompressed;
	const size_t	mDataMaxSize;
	size_t			mDataSize;
	intptr_t		mDataOffset;
	char *			mData;
};

// host to network short
// u_short htons(u_short hostshort); // convert from host byte order to network byte order to 16 bits integer 

// host to network long
// u_long htonl(u_long hostlong); // convert from host byte order to network byte order to 32 bits integer 

typedef crNetMessage msg_t;

inline constexpr uint32_t MAX_INTERFACES = 32;

struct sockaddr_in;
class crNetwork
{
public:
    static crNetwork*  Get( void );

	crNetwork( void );

    virtual void			Init( void ) = 0;
    virtual void			Shutdown( void ) = 0;

	virtual int				IPSocket( const char* bind_ip, int port, netadr_t* bound_to ) = 0;
	virtual bool			GetUDPPacket( int netSocket, netadr_t& net_from, char* data, size_t& size, size_t maxSize ) = 0;
	virtual bool			WaitForData( int netSocket, int timeout ) = 0;
	virtual void			SendUDPPacket( int netSocket, size_t length, const void* data, const netadr_t to ) = 0;

    // parses the port number
    // can also do DNS resolve if you ask for it.
    // NOTE: DNS resolve is a slow/blocking call, think before you use
    // ( could be exploited for server DoS )
    bool					StringToNetAdr( const char* s, netadr_t* a, bool doDNSResolve );

    const char* 			NetAdrToString( const netadr_t a );
    bool					IsLANAddress( const netadr_t a );
    bool					CompareNetAdrBase( const netadr_t a, const netadr_t b ) const; //( can be a static member )
    uint32_t				GetLocalIPCount( void ) const;
    const char* 			GetLocalIP( int i ) const;

protected:
	static idCVar net_socksServer;
	static idCVar net_socksPort;
	static idCVar net_socksUsername;
	static idCVar net_socksPassword;
	static idCVar net_ip;

	typedef struct
	{
		// RB: 64 bit fixes, changed long to int
		// FIXME: IPv6?
		uint32_t ip;
		uint32_t mask;
		// RB end
		char addr[16];
	} net_interface;

	uint32_t			m_numInterfaces;
	net_interface		m_netint[MAX_INTERFACES];
	char				m_socksBuf[4096];
	struct sockaddr_in	m_socksRelayAddr;

	bool	ExtractPort( const char* src, char* buf, const size_t bufsize, int* port );
	bool	StringToSockaddr( const char* s, sockaddr_in* sadr, const bool doDNSResolve );
	void	SockadrToNetadr( sockaddr_in* s, netadr_t* a );
	void	NetadrToSockadr( const netadr_t* a, sockaddr_in* s );

	/// PLATFOM SPECIFIC
	virtual const char* ErrorString( void ) const = 0;
};

#endif //!__NETWORK_SYSTEM_HPP__