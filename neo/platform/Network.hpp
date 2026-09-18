
#ifndef __NETWORK_SYSTEM_HPP__
#define __NETWORK_SYSTEM_HPP__

inline constexpr uint16_t PORT_ANY = 0;
typedef uint16_t portID_t;

/*
==============================================================

	Networking

==============================================================
*/

typedef struct NET_Address NET_Address;
typedef struct NET_DatagramSocket NET_DatagramSocket;

typedef enum : uint8_t
{
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,					// IPv4
	NA_IP6					// IPv6
} netadrtype_t;

class crAddress
{
public:
	crAddress( void );
	crAddress( const netadrtype_t in_type, const portID_t in_port );
	crAddress( const idStr in_from, const portID_t in_port );
	crAddress( const crAddress &in_ref );
	~crAddress( void );

	netadrtype_t	Type( void ) const { return m_type; }
	portID_t		Port( void ) const { return m_port; }

	void			GetAnderess( uint8_t * out_bytes ) const;
	const char*		ToString( void ) const;

	crAddress operator = ( const crAddress & in_ref );
	bool operator == ( const crAddress & in_ref ) const;

protected:
	friend class idUDP;
	friend class crNetwork;
	crAddress( NET_Address* in_addrs, const portID_t in_port );
	NET_Address*	GetHandle( void ) const { return m_address; }	

private:
	netadrtype_t	m_type;		// the conection type 
	portID_t		m_port;		// the port that conect 
	NET_Address*	m_address;	// opaque handle to SDL3_net library
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
	idUDP( void );
	virtual		~idUDP( void );
	
	// if the InitForPort fails, the idUDP.port field will remain 0
	bool		InitForPort( const portID_t in_portNumber );
	
	uint16_t	GetPort( void ) const { return m_bound.Port(); }

	crAddress	GetAdr( void ) const { return m_bound; }

	uint32_t		GetUIntAdrIPV4( void ) const
	{
		return 0; // TODO:
		//return ( bound_to.ip[0] | bound_to.ip[1] << 8 | bound_to.ip[2] << 16 | bound_to.ip[3] << 24 );
	}

	void		Close( void );
	
	bool		GetPacket( crAddress& out_from, void* out_data, size_t& out_size, size_t in_maxSize );
	
	bool		GetPacketBlocking( crAddress& out_from, void* out_data, size_t& out_size, const size_t out_maxSize, const int32_t out_timeout );
								   
	void		SendPacket( const crAddress &in_to, const void* in_data, const size_t in_size );
	
	void		SetSilent( const bool in_silent )
	{
		m_silent = in_silent;
	}

	bool		GetSilent( void ) const
	{
		return m_silent;
	}
	
	uint32_t	m_packetsRead;
	size_t		m_bytesRead;
	
	uint32_t	m_packetsWritten;
	size_t		m_bytesWritten;
	
	bool		IsOpen( void ) const
	{
		return m_netSocket != nullptr;
	}
	
private:
	crAddress			m_bound;		// interface and port
	bool				m_silent;			// don't emit anything ( black hole )
	bool				m_isInitialized;
	NET_DatagramSocket*	m_netSocket;
};

class crNetMessage
{
public:
	crNetMessage( const size_t bufferSize = 32767, const bool compressed = true );
	~crNetMessage( void );

	void ResetReadOffset();

	bool ReadBytes(char * bytes, size_t numBytes);

	template<typename T>
	bool Read(T & output)
	{
		return ReadBytes((char*)&output, sizeof(T));
	}

	bool ReadString(char * buffer, size_t maxlength);

	bool WriteBytes(const char * bytes, size_t numBytes);

	template<typename T>
	bool Write(const T & output)
	{
		return WriteBytes((const char *)&output, sizeof(T));
	}

	bool WriteString(const char * output);

	bool ReadPacket(idUDP & socket, crAddress & addrFrom);
	void SendPacket(idUDP & socket, const crAddress & addr);
	
	inline const char * Data ( void ) const{ return mData; }
	inline size_t Size( void ) const { return mDataOffset; }


private:
	const bool		mCompressed;
	const size_t	mDataMaxSize;
	size_t			mDataSize;
	intptr_t		mDataOffset;
	char *			mData;
};

typedef crNetMessage msg_t;

inline constexpr uint32_t MAX_INTERFACES = 32;

class crNetwork
{
public:
    static crNetwork*  Get( void );

	crNetwork( void );
	void					Init( void );
    void					Shutdown( void );
    bool					IsLANAddress( const crAddress &a );
    uint32_t				GetLocalIPCount( void ) const;
    const char* 			GetLocalIP( const uint32_t i ) const;

protected:

#if 0
	static idCVar net_socksServer;
	static idCVar net_socksPort;
	static idCVar net_socksUsername;
	static idCVar net_socksPassword;
	static idCVar net_ip;
#endif

	int          		m_localAddressCount;
	crAddress*   		m_localAddresses;

	bool	ExtractPort( const char* src, char* buf, const size_t bufsize, int* port );
};

#endif //!__NETWORK_SYSTEM_HPP__