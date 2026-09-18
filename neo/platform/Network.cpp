
#include "precompiled.h"
#include "Network.hpp"

#include <SDL3_net/SDL_net.h>

static idCVar net_socksServer( "net_socksServer", "", CVAR_ARCHIVE, "" );
static idCVar net_socksPort( "net_socksPort", "1080", CVAR_ARCHIVE | CVAR_INTEGER, "" );
static idCVar net_socksUsername( "net_socksUsername", "", CVAR_ARCHIVE, "" );
static idCVar net_socksPassword( "net_socksPassword", "", CVAR_ARCHIVE, "" );
static idCVar net_ip( "net_ip", "localhost", CVAR_NOCHEAT, "local IP address" );

static void ip_to_addr( const char ip[4], char* addr )
{
	idStr::snPrintf( addr, 16, "%d.%d.%d.%d", ( unsigned char )ip[0], ( unsigned char )ip[1],
					 ( unsigned char )ip[2], ( unsigned char )ip[3] );
}

/*
================================================================================================
crAddress
================================================================================================
*/

/*
========================
crAddress::crAddress
========================
*/
crAddress::crAddress( void ) :
	m_type( NA_BAD ), 
	m_address( nullptr ),
	m_port( PORT_ANY )
{
}

/*
========================
crAddress::crAddress
========================
*/
crAddress::crAddress( const netadrtype_t in_type, const portID_t in_port ) : 
	m_type( in_type ),
	m_port( in_type ),
	m_address( nullptr ) 
{
    m_port = in_port;
	idassert( in_type == NA_BROADCAST || in_type == NA_LOOPBACK );
	
	// Instead of calling NET_ResolveHostname for fixed strings, we keep it as nullptr. 
	// idUDP::SendPacket will handle translating the nullptr at the time of sending!
	/*
	if ( in_type == NA_BROADCAST ) 
        m_address = NET_ResolveHostname( "255.255.255.255" );
	else if ( in_type == NA_LOOPBACK ) 
        m_address = NET_ResolveHostname( "127.0.0.1" );

    if ( m_address )
        NET_WaitUntilResolved( m_address, -1 );
	*/
}

/*
========================
crAddress::crAddress
========================
*/
crAddress::crAddress( const idStr in_from, const portID_t in_port ) : 
	m_type( NA_BAD ),
	m_port( 0 ),
	m_address( nullptr )
{
	// If are specific IP, resolve it 
	if ( ( !in_from.IsEmpty() ) && ( in_from.Cmp( "localhost" ) != 0 ) ) 
	{
        m_address = NET_ResolveHostname( in_from );
        if ( m_address ) 
		{
            // Ensures immediate resolution because it is numerical.
            NET_WaitUntilResolved( m_address, -1 ); //
            
            // Validates whether the resolution actually failed.
            if ( NET_GetAddressStatus( m_address ) == NET_FAILURE ) 
			{ 
				m_type = NA_BAD;
                idLib::Printf( "crAddress::OpenFromString: failed to resolve '%s'\n", in_from );
                NET_UnrefAddress( m_address ); 
                return;
            }

			int numBytes = 0;
        	NET_GetAddressBytes( m_address, &numBytes );
        	m_type = ( numBytes == 16 ) ? NA_IP6 : NA_IP;
        }
		else 
		{
        	m_type = NA_BAD;
    	}
    }	
}

/*
========================
crAddress::crAddress
========================
*/
crAddress::crAddress( NET_Address *in_addrs, const portID_t in_port) : m_port( in_port ), m_address( in_addrs )
{
	int numBytes = 0;
	
	if ( !m_address ) //( may can be a loopback ? )
	{
		m_type = NA_BAD;
		return; // throw a exeption ?
	}

    // increment reference the address pointer 
    NET_RefAddress( m_address ); 
        
    NET_GetAddressBytes( m_address, &numBytes );
    
	// Retrieve the type
	m_type = ( numBytes == 16 ) ? NA_IP6 : NA_IP;
}

/*
========================
crAddress::crAddress
========================
*/
crAddress::crAddress( const crAddress &in_ref ) :
	m_type( in_ref.m_type ),
	m_port( in_ref.m_port ),	
	m_address( in_ref.m_address )
{
	// we are referencing a already existing pointer 
	if( m_address != nullptr )
		NET_RefAddress( m_address );
}

/*
========================
crAddress::~crAddress
========================
*/
crAddress::~crAddress(void)
{
	// decrement reference 
	if ( m_address )
	{
		// release this reference
		NET_UnrefAddress( m_address );
		m_address = nullptr; 
	}

	m_port = 0;
	m_type = NA_BAD;
}

/*
========================
crAddress::GetAnderess
========================
*/
void crAddress::GetAnderess( uint8_t *bytes ) const
{
	int numBytes = 0;
	if( !m_address || m_type == NA_BAD )
		return;

	// Gets the pointer to the raw network bytes stored by SDL3
    const uint8_t *addressBytes = (const uint8_t *)NET_GetAddressBytes( m_address, &numBytes ); //
    if ( addressBytes != nullptr ) 
	{
        if ( numBytes == 4 ) 
		{
			// Raw IPv4: Copies the 4 bytes directly into the idTech 4 structure.
			SDL_memcpy( &bytes[0], &addressBytes[0], numBytes );
        } 
        else if ( numBytes == 16 ) 
		{
            // If SDL3_net receives an IPv4 address mapped within IPv6 (OS Dual-Stack mechanism)
            // The last 4 bytes of an IPv4-mapped IPv6 address (::ffff:192.168.x.x) represent the actual IPv4 address.
			if ( addressBytes[10] == 0xFF && addressBytes[11] == 0xFF ) 
				SDL_memcpy( &bytes[0], &addressBytes[12], 4 );
			else 
            	// True native IPv6. 
				SDL_memcpy( &bytes[0], &addressBytes[0], numBytes );
			
        }
    } 
	else 
	{
        SDL_memset( bytes, 0x00, 16 );
    }
}


// DG: FIXME: those static buffers look fishy - I would feel better if they were
//            at least thread-local - so /maybe/ use ID_TLS here?
//            or maybe return an idStr and change calling code accordingly
static int index = 0;	// todo atomic
static char buf[ 32 ][ 64 ];
// flip/flop

/*
========================
crAddress::ToString
========================
*/
const char *crAddress::ToString(void) const
{
	/// this will continue valid, wile address exist 
	auto local = NET_GetAddressString( m_address );
	
	char* s = buf[index];
	index = ( index + 1 ) & 3;
    
	// copy to our temp string 
	SDL_strlcpy( s, local, SDL_strnlen( local, 64 ) );

	return s;
}

/*
========================
crAddress::operator =
========================
*/
crAddress crAddress::operator=(const crAddress &in_ref)
{
	// Protect from auto referencing 
	if ( this == &in_ref )
        return *this;	

	// if any current address reference in the class, drop it
	if( m_address != nullptr )
		NET_UnrefAddress( m_address );

	// copy the reference 
	m_address = in_ref.m_address;
	m_port = in_ref.m_port;
	m_type = in_ref.m_type;

	// 
	if( m_address != nullptr )
		NET_RefAddress( m_address );

	return *this;
}

/*
========================
crAddress::operator ==
========================
*/
bool crAddress::operator==(const crAddress &in_ref) const
{
    return NET_CompareAddresses( m_address, in_ref.m_address );
}

/*
================================================================================================
idUDP
================================================================================================
*/

/*
========================
idUDP::idUDP
========================
*/
idUDP::idUDP( void ) : 
	m_bound(),
	m_netSocket( nullptr ),
	m_silent( false ),
	m_packetsRead( 0 ),
	m_bytesRead( 0 ),
	m_packetsWritten( 0 ),
	m_bytesWritten( 0 )
{
}

/*
========================
idUDP::~idUDP
========================
*/
idUDP::~idUDP( void )
{
	Close();
}

/*
========================
idUDP::InitForPort
========================
*/
bool idUDP::InitForPort( const portID_t in_portNumber )
{
	// Close any previous socket 
	if ( m_netSocket != nullptr ) 
	{
        NET_DestroyDatagramSocket( m_netSocket );
        m_netSocket = nullptr;
    }

	// Configure modern socket properties via SDL_PropertiesID
	// We enable address reuse (useful for server crashes/quick reboots)
	// and explicitly allow broadcasting (essential for LAN detection in idTech 4).
	SDL_PropertiesID props = SDL_CreateProperties();
    if ( props != 0 ) 
	{
        SDL_SetBooleanProperty( props, NET_PROP_DATAGRAM_SOCKET_ALLOW_BROADCAST_BOOLEAN, true );
        SDL_SetBooleanProperty( props, NET_PROP_DATAGRAM_SOCKET_REUSEADDR_BOOLEAN, true );
    }	

	// Creates the Datagram (UDP) socket
	// Passing nullptr binds to "any" (0.0.0.0 and ::), automatically enabling dual-stack. 
	// SDL3_net internally converts portNumber to the correct network byte order.
	m_netSocket = NET_CreateDatagramSocket( nullptr, in_portNumber, props );

	// done release properties handle 
	if ( props != 0 ) 
	    SDL_DestroyProperties( props );
    

	if ( !m_netSocket ) 
	{
		m_bound = crAddress();
		idLib::Printf( "idUDP::Init: Failed to open port %d: %s\n", m_portNumber, SDL_GetError() );
        return false;
    } 

	// Resets the idTech 4 network statistical counters for this new session.
    m_packetsRead = 0;
    m_bytesRead = 0;
    m_packetsWritten = 0;
    m_bytesWritten = 0;

	idLib::Printf( "idUDP::InitForPort: Port %d successfully opened in Dual-Stack mode. (IPv4/IPv6).\n", portNumber );

	return true;
}

/*
========================
idUDP::Close
========================
*/
void idUDP::Close( void )
{
	if( m_netSocket )
	{
		NET_DestroyDatagramSocket( m_netSocket );
		m_netSocket = nullptr;
		m_bound = crAddress();
	}
}

/*
========================
idUDP::GetPacket
========================
*/
bool idUDP::GetPacket( crAddress& out_from, void* out_data, size_t& out_size, const size_t in_maxSize )
{
	NET_Datagram *packet = nullptr;

	// assecure the size to 0 if we can't read any packet 
	out_size = 0;

	// Verify if our SDL3_net socket is active.
	if ( !m_netSocket )
		return false;

	// attempts to retrieve a datagram from the SDL3 asynchronous queue
	if ( !NET_ReceiveDatagram( m_netSocket, &packet ) ) 
		return false;
	
	// somenthing wrong
    if ( !packet )
	{
		return false;
		idLib::Printf( "GetPacket: %s\n", SDL_GetError() );
	}

    //
	// Clamps the size to prevent a buffer overflow if the packet is larger than expected.
    out_size = Min( in_maxSize, (size_t)packet->buflen );
	
    // Copies the raw bytes to the engine's data buffer.
    std::memcpy( out_data, packet->buf, out_size );

	// update Address reference
	out_from = crAddress( packet->addr, packet->port );
	
    // Release SDL Datagram, to prevent memory leak
    NET_DestroyDatagram( packet );
    
	m_packetsRead++;
	m_bytesRead += out_size;
	
	return true;
}

/*
========================
idUDP::GetPacketBlocking
========================
*/
bool idUDP::GetPacketBlocking( crAddress& out_from, void* out_data, size_t& out_size, const size_t in_maxSize, const int32_t in_timeout )
{
	NET_Datagram *packet = nullptr;
    out_size = 0;

	if ( !m_netSocket || !out_data ) 
        return false;

	// Attempts to read immediately if a packet is already queued in SDL3 memory
	if ( NET_ReceiveDatagram( m_netSocket, &packet ) ) 
	{
        if ( packet ) 
		{
			out_size = Min( in_maxSize, ( size_t ) packet->buflen );
            std::memcpy( out_data, packet->buf, out_size );
            out_from = crAddress( packet->addr, packet->port );
            NET_DestroyDatagram( packet );
            
			m_packetsRead++;
			m_bytesRead += out_size;
        }
    }

    // If the internal queue was empty, we force an operating system block.
    // Since NET_WaitUntilInputAvailable accepts a generic array (void**), we create a single-element array.
	void *socketArray[1] = { (void*)m_netSocket };
    
	// If the timeout_ms parameter is less than 0 in the original call, we pass -1 (infinite wait)
    Sint32 sdlTimeout = ( in_timeout < 0 ) ? -1 : (Sint32)in_timeout;

	// Puts the engine thread to sleep until data is received or the timeout expires.
    // Returns > 0 if the socket has data available.
	int readySockets = NET_WaitUntilInputAvailable( socketArray, 1, sdlTimeout );
	if( readySockets <= 0 )
		return false;

    if ( !NET_ReceiveDatagram( m_netSocket, &packet ) )
		return false;
		
    if ( !packet ) // TODO: print a error
		return false;
		
	out_size = Min( in_maxSize, (size_t)packet->buflen );
	std::memcpy( out_data, packet->buf, out_size );
	out_from = crAddress( packet->addr, packet->port );
	NET_DestroyDatagram( packet );

	m_packetsRead++;
	m_bytesRead += out_size;

	return false;
}

/*
========================
idUDP::SendPacket
========================
*/
void idUDP::SendPacket( const crAddress &in_to, const void* in_data, const size_t in_size )
{
	if( in_to.Type() == NA_BAD )
	{
		idLib::Warning( "idUDP::SendPacket: bad address type NA_BAD - ignored" );
		return;
	}
		
	// Verify if our SDL3_net socket is active.
	if ( !m_netSocket || m_silent )
		return;
	
	// Resolves the correct handle
	NET_Address* targetHandle = in_to.GetHandle();
	
	// Special addresses (Broadcast and Loopback) must pass nullptr!
	if ( in_to.Type() == NA_BROADCAST || in_to.Type() == NA_LOOPBACK ) 
		targetHandle = nullptr; 

	// Sends asynchronously. SDL places this in an internal queue and dispatches it.
	if ( !NET_SendDatagram( m_netSocket, in_to.GetHandle(), in_to.Port(), in_data, in_size ) )
	{ 
		idLib::Printf( "idUDP::SendPacket sendto error - packet dropped: %s\n", SDL_GetError() );
		return;
	}

	m_packetsWritten++;
	m_bytesWritten += in_size;
}

/*
================================================================================================
crNetMessage
================================================================================================
*/

/*
========================
crNetMessage::crNetMessage
========================
*/
crNetMessage::crNetMessage( const size_t bufferSize, const bool compressed ) : 
    mData(0)
	, mDataOffset(0)
	, mDataSize(0)
	, mDataMaxSize(bufferSize)
	, mCompressed(compressed)
{
	mData = new char[mDataMaxSize];
	std::memset(mData, 0, sizeof(mData));
}

/*
========================
crNetMessage::~crNetMessage
========================
*/
crNetMessage::~crNetMessage( void )
{
	delete[] mData;
}

void crNetMessage::ResetReadOffset() 
{
	mDataOffset = 0; 
}

bool crNetMessage::ReadBytes( char * bytes, size_t numBytes )
{
	assert(!(mDataOffset + numBytes > mDataSize));
	if (mDataOffset + numBytes > mDataSize)
		return false;

	char * ptr = mData + mDataOffset;
	mDataOffset += numBytes;
	std::memcpy(bytes, ptr, numBytes);
	return true;
}

bool crNetMessage::ReadString(char * buffer, size_t maxlength )
{
	unsigned short length = 0;
	return Read(length) && ReadBytes(buffer, length);
}

bool crNetMessage::WriteBytes(const char * bytes, size_t numBytes )
{
	assert(!(mDataOffset + numBytes > mDataMaxSize));
	if (mDataOffset + numBytes > mDataMaxSize)
		return false;

	char * ptr = mData + mDataOffset;
	mDataOffset += numBytes;
	std::memcpy(ptr, bytes, numBytes);
	return true;
}

bool crNetMessage::WriteString(const char * output)
{
	const unsigned short length = (unsigned short)strlen(output) + 1;
	return Write(length) && WriteBytes(output, length);
}

bool crNetMessage::ReadPacket(idUDP & socket, crAddress & addrFrom)
{
	mDataOffset = 0;
	if (mCompressed)
	{
		void * tmpBuffer = _alloca(mDataMaxSize);

		size_t packetSize = 0;
		if (socket.GetPacket(addrFrom, tmpBuffer, packetSize, mDataMaxSize))
		{
			idFile_Memory fmem("compressed", (const char*)tmpBuffer, packetSize);
			
			mDataSize = 0;
            int size = 0;
			if (fmem.ReadInt( size ) && packetSize <= mDataMaxSize)
			{
// BEATO Begin:
                mDataSize = size;
				crStaticPointer<idCompressor> compressor(idCompressor::AllocLZSS());
// BEATO End
				compressor->Init(&fmem, false, 8);
				if (compressor->Read(mData, mDataSize))
					return true;
			}
		}
		return false;
	}
	else
	{
		return socket.GetPacket(addrFrom, mData, mDataSize, mDataMaxSize);
	}
}

void crNetMessage::SendPacket(idUDP & socket, const crAddress & addr)
{
	if (mCompressed)
	{
		idFile_Memory fmem("compressed");

		// write the uncompressed size
		fmem.WriteInt(Size());

// BEATO Begin:
#if 0
		std::auto_ptr<idCompressor> compressor(idCompressor::AllocLZSS());
#else	
		crStaticPointer<idCompressor> compressor(idCompressor::AllocLZSS());
#endif
		compressor->Init(&fmem, true, 8);
		compressor->Write(Data(), Size());
		compressor->FinishCompress();

		socket.SendPacket(addr, fmem.GetDataPtr(), fmem.Length());
	}
	else
	{
		socket.SendPacket(addr, mData, mDataOffset);
	}
}

/*
================================================================================================
crNetwork
================================================================================================
*/

/*
========================
crNetwork::crNetwork
========================
*/
crNetwork::crNetwork(void)
{
}

void crNetwork::Init( void )
{
	int count = 0;

	// Try initialize SDL3_net
	if ( !NET_Init() ) 
    	idLib::Error(" Failed to initialize SDL3_net: %s", SDL_GetError()); 

	// List all local addresses 
	NET_Address** sdlAddresses = NET_GetLocalAddresses( &count );
	if ( sdlAddresses != nullptr && count > 0 ) 
	{
        m_localAddressCount = count;
        m_localAddresses = new crAddress[m_localAddressCount];

        for ( int i = 0; i < count; i++ ) 
		{
            // We pass port 0 by default for pure local IPs.
            m_localAddresses[i] = crAddress( sdlAddresses[i], 0 );
        }

        // Release address array
        SDL_free( sdlAddresses ); // 
    }	
}

void crNetwork::Shutdown(void)
{
	if( m_localAddresses != nullptr )
	{
		delete[] m_localAddresses;
		m_localAddresses = nullptr;
	}

	NET_Quit();
}

/*
========================
crNetwork::IsLANAddress
========================
*/
bool crNetwork::IsLANAddress( const crAddress &adr )
{
	if ( adr.Type() == NA_LOOPBACK || adr.Type() == NA_BROADCAST ) 
        return true;
	
	uint8_t ip[16];
    SDL_memset( ip, 0, sizeof( ip ) );
    adr.GetAnderess( ip );

    // --- IPv4 VERIFICATION ---
    if ( adr.Type() == NA_IP ) 
	{
        // Localhost / Classic loopback
		if ( ip[0] == 127 ) 
			return true;
        
        // Private Class A: 10.0.0.0 - 10.255.255.255
		if ( ip[0] == 10 ) 
			return true;
        
        // Private Class B: 172.16.0.0 - 172.31.255.255
		if ( ip[0] == 172 && ( ip[1] >= 16 && ip[1] <= 31 ) ) 
			return true;
        
        // Private Class C: 192.168.0.0 - 192.168.255.255
		if ( ip[0] == 192 && ip[1] == 168 ) 
			return true;
        
        // Link-Local (Autoconfiguration / No Router): 169.254.0.0 - 169.254.255.255
		if ( ip[0] == 169 && ip[1] == 254 ) 
			return true;
        
        return false;
    }

    // --- IPv6 VERIFICATION ---
    if ( adr.Type() == NA_IP6 ) 
	{
        // IPv6 Loopback (::1) -> The first 15 bytes are 0 and the last one is 1
		bool isLoopback = true;
        for ( int i = 0; i < 15; i++ ) 
		{
            if ( ip[i] != 0 ) 
			{
                isLoopback = false;
                break;
            }
        }

        if ( isLoopback && ip[15] == 1 ) 
			return true;

		// Link-Local IPv6: Starts with fe80:: (0xFE and the first bits of the second byte as 0x80)
        if ( ip[0] == 0xFE && ( ip[1] & 0xC0 ) == 0x80 ) 
			return true;

        // Unique Local Address (ULA): Starts with fc00:: or fd00:: (0xFC ou 0xFD)
        if ( ip[0] == 0xFC || ip[0] == 0xFD ) 
			return true;

        return false;
    }

    return false;
}

/*
========================
crNetwork::GetLocalIPCount
========================
*/
uint32_t crNetwork::GetLocalIPCount( void ) const
{
	if( m_localAddressCount == 0 && m_localAddresses == nullptr )
		return 0;

	return m_localAddressCount;
}

/*
========================
crNetwork::GetLocalIP
========================
*/
const char* crNetwork::GetLocalIP( const uint32_t i ) const
{
	if( ( i >= m_localAddressCount ) )
		return nullptr;

	return m_localAddresses[i].ToString();
}


/*
========================
crNetwork::ExtractPort
========================
*/
bool crNetwork::ExtractPort( const char* in_src, char* in_buf, const size_t in_size, int* port )
{
	char* p = nullptr;
#if CR_USE_SDL_STRING_UTILS
	SDL_strlcpy( buf, src, bufsize ); // TODO: check for bugs 
#else
	std::strncpy( buf, src, bufsize );
#endif
	p = buf;
	p += Min( bufsize - 1, idStr::Length( src ) );
	*p = '\0';

#if CR_USE_SDL_STRING_UTILS
    p = SDL_strchr( buf, ':' );
#else
    p = std::strchr( buf, ':' );
#endif

	if( !p )
		return false;
	
	*p = '\0';

#if CR_USE_SDL_STRING_UTILS
    long lport = SDL_strtol( p + 1, nullptr, 10 );
#else
	long lport = std::strtol( p + 1, nullptr, 10 );
#endif

	if( lport == 0 || lport == LONG_MIN || lport == LONG_MAX )
	{
		*port = 0;
		return false;
	}
	*port = lport;
	return true;
}
