
#include "precompiled.h"
#include "Network.hpp"

#define USE_SDL3NET 1

#if USE_SDL3NET
#include <SDL3_net/SDL_net.h>
#elif __PLATFORM_LINUX__
#   include <arpa/inet.h>
#   include <netdb.h>
#elif __PLATFORM_WINDOWS__
#   include <winsock2.h>
#   include <ws2tcpip.h>
#endif

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

#if 0
static void IdAddressToSDLAddress( const netadr_t &idAddr, NET_Address **sdlAddr, uint16_t *port ) 
{
	// Na idTech 4 as portas na netadr_t já estão em Host Byte Order (normalmente)
    *port = idAddr.port; 

    if ( idAddr.type == NA_BROADCAST ) 
	{
        // Endereço de broadcast padrão
        *sdlAddr = NET_ResolveHostname("255.255.255.255");
    } 
	else if ( idAddr.type == NA_LOOPBACK ) 
	{
        *sdlAddr = NET_ResolveHostname("127.0.0.1");
    } 
	else 
	{
        // Converte os 4 bytes brutos para o formato de string clássico "a.b.c.d"
        char ipStr[32];
        SDL_snprintf( ipStr, sizeof(ipStr), "%d.%d.%d.%d", 
                      idAddr.ip[0], idAddr.ip[1], idAddr.ip[2], idAddr.ip[3] );
        
        // Resolve de forma síncrona/imediata já que é um IP numérico puro
        *sdlAddr = NET_ResolveHostname( ipStr );
    }

    // Como o SDL3_net resolve assincronamente por padrão, forçamos o travamento 
    // imediato. Como é um IP cru (sem DNS), isso resolve instantaneamente (0ms)
    if ( *sdlAddr ) 
	{
        NET_WaitUntilResolved( *sdlAddr, -1 ); //
    }
}
#endif

static void SDLAddressToIdAddress( NET_Address *sdlAddr, uint16_t port, netadr_t &idAddr ) 
{
    idAddr.port = port;
    idAddr.type = NA_IP; // Default for packets coming from the network

    int numBytes = 0;
	
	// Gets the pointer to the raw network bytes stored by SDL3
    const uint8_t *bytes = (const uint8_t *)NET_GetAddressBytes( sdlAddr, &numBytes ); //
    if ( bytes != nullptr ) 
	{
        if ( numBytes == 4 ) 
		{
			// Raw IPv4: Copies the 4 bytes directly into the idTech 4 structure.
			SDL_memcpy( &idAddr.ip[0], &bytes[0], numBytes );
        } 
        else if ( numBytes == 16 ) 
		{
            // If SDL3_net receives an IPv4 address mapped within IPv6 (OS Dual-Stack mechanism)
            // The last 4 bytes of an IPv4-mapped IPv6 address (::ffff:192.168.x.x) represent the actual IPv4 address.
			if ( bytes[10] == 0xFF && bytes[11] == 0xFF ) 
			{
				SDL_memcpy( &idAddr.ip[0], &bytes[12], 4 );
            } 
			else 
			{
            	// True native IPv6. 
			    idAddr.type = NA_IP6;
				SDL_memcpy( &idAddr.ip[0], &bytes[0], numBytes );
			}
        }
    } 
	else 
	{
        idAddr.type = NA_BAD;
        SDL_memset( idAddr.ip, 0, 4 );
    }
}

/*
========================
NET_IPSocket
========================
*/
static NET_DatagramSocket* NET_IPSocket( const char* bind_ip, Uint16 port, netadr_t* bound_to ) 
{
    NET_Address* bindAddr = nullptr;

    // Se um IP específico foi passado, precisamos tratá-lo
    // If are specific IP, resolve it 
	if ( bind_ip && bind_ip[0] && idStr::Icmp( bind_ip, "localhost" ) != 0 ) 
	{
        bindAddr = NET_ResolveHostname( bind_ip );
        if ( bindAddr ) 
		{
            // Ensures immediate resolution because it is numerical.
            NET_WaitUntilResolved( bindAddr, -1 ); //
            
            // Validates whether the resolution actually failed.
            if ( NET_GetAddressStatus( bindAddr ) == NET_FAILURE ) 
			{ 
                idLib::Printf( "NET_IPSocket: Falha ao resolver bind_ip '%s'\n", bind_ip );
                NET_UnrefAddress( bindAddr ); 
                return nullptr;
            }
        }
    }

    // NOTE: If bindAddr remains nullptr, SDL3_net will bind to "any" (0.0.0.0 and ::)

	// Create and associate the Socket using modern SDL3 properties
	// We use SDL_CreateProperties() if you want to customize (e.g., REUSEADDR or ALLOW/_BROADCAST)
	// If you don't need additional customizations, the last parameter can simply be 0.
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetBooleanProperty( props, NET_PROP_DATAGRAM_SOCKET_ALLOW_BROADCAST_BOOLEAN, true ); //
    SDL_SetBooleanProperty( props, NET_PROP_DATAGRAM_SOCKET_REUSEADDR_BOOLEAN, true );       //

    NET_DatagramSocket* socket = NET_CreateDatagramSocket( bindAddr, port, props ); //

	// Clears temporary properties and the resolved address
    SDL_DestroyProperties( props );
    if ( bindAddr ) 
        NET_UnrefAddress( bindAddr ); //

    if ( !socket ) 
	{
        common->Printf( "NET_IPSocket: Failed to create soket on port %d: %s\n", port, SDL_GetError() );
        return nullptr;
    }

    // Populate the netadr_t structure with the actual address to which we were bound (bound_to).
    if ( bound_to ) 
	{
        SDL_memset( bound_to, 0, sizeof( netadr_t ) );
        
		// If bound to a specific IP, we retrieve its bytes.
        // If bound to "any", the OS will report zeros, but we set the appropriate type.
		if ( bind_ip && bind_ip[0] ) 
		{
            // We use the conversion function created in the previous step.
			// Note that we need to retrieve the actual bound address from the API if the OS modified it,
			// but since SDL3_net abstracts this, we will read the original bindAddr or assume the default.
			// For greater robustness with random ports (port == 0):
			bound_to->port = port; 
            bound_to->type = ( idStr( bind_ip ).Find( ':' ) != -1 ) ? NA_IP6 : NA_IP;

        } 
		else 
		{
            // Dual-stack local listener por padrão opera em IPv6 recebendo também IPv4 mapeado
            bound_to->type = NA_IP6;
            bound_to->port = port;
        }

        // If the engine opened on port 0, the OS chose a dynamic port.
        // Since SDL3_net lacks a direct public "NET_GetSocketPort" method,
        // if you are using dynamic ports in idTech 4 (such as for dedicated server listening ports),
        // ensure you pass the static port configured in the "net_port" CVar (default 27666).
    }

    return socket;
}

/*
================================================================================================
crAddress
================================================================================================
*/

void crAddress::OpenFromString(const idStr in_from, const uint16_t in_port)
{
}

// DG: FIXME: those static buffers look fishy - I would feel better if they were
//            at least thread-local - so /maybe/ use ID_TLS here?
//            or maybe return an idStr and change calling code accordingly
static int index = 0;	// todo atomic
static char buf[ 32 ][ 64 ];	// flip/flop
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
idUDP::idUDP( void )
{
	m_netSocket = nullptr;
	std::memset( &bound_to, 0, sizeof( bound_to ) );
	silent = false;
	packetsRead = 0;
	bytesRead = 0;
	packetsWritten = 0;
	bytesWritten = 0;
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
bool idUDP::InitForPort( int portNumber )
{

#if USE_SDL3NET
	m_netSocket = NET_IPSocket( nullptr, portNumber, &bound_to );
	if ( !m_netSocket ) 
	{
		std::memset( &bound_to, 0, sizeof( bound_to ) );
        idLib::Printf( "idUDP::Init: Failed to open port %d: %s\n", portNumber, SDL_GetError() );
        return false;
    }
#else
	// DG: don't specify an IP to bind for (and certainly not net_ip)
	// => it'll listen on all addresses (0.0.0.0 / INADDR_ANY)
	netSocket = crNetwork::Get()->IPSocket( nullptr, portNumber, &bound_to );
	// DG end
	if( netSocket <= 0 )
	{
		netSocket = 0;
		std::memset( &bound_to, 0, sizeof( bound_to ) );
		return false;
	}
#endif 

	return true;
}

/*
========================
idUDP::Close
========================
*/
void idUDP::Close( void )
{
#if USE_SDL3NET
	if( m_netSocket )
	{
		NET_DestroyDatagramSocket( m_netSocket );
		m_netSocket = nullptr;
		std::memset( &bound_to, 0, sizeof( bound_to ) );
	}
#else
	if( netSocket )
	{		
#if __PLATFORM_LINUX__
        close( netSocket );
#else if __PLATFORM_WINDOWS__
		closesocket( netSocket );
#endif
		netSocket = 0;
		std::memset( &bound_to, 0, sizeof( bound_to ) );
	}
#endif
}

/*
========================
idUDP::GetPacket
========================
*/
bool idUDP::GetPacket( crAddress& from, void* data, size_t& size, size_t maxSize )
{
	NET_Datagram *packet = nullptr;

	// Verify if our SDL3_net socket is active.
	if ( !m_netSocket )
		return false;

	// attempts to retrieve a datagram from the SDL3 asynchronous queue
	if ( !NET_ReceiveDatagram( m_netSocket, &packet ) ) 
		idLib::Printf( "GetPacket: %s\n", SDL_GetError() );
	
	// somenthing wrong
    if ( !packet )
		return false;

    //
	// Clamps the size to prevent a buffer overflow if the packet is larger than expected.
    size = ( packet->buflen > maxSize ) ? maxSize : packet->buflen;

    // Copies the raw bytes to the engine's data buffer.
    std::memcpy( data, packet->buf, size );

    // Converts the SDL NET_Address to the idTech 4 netadr_t.
	SDLAddressToIdAddress( packet->addr, packet->port, from );

    // Release SDL Datagram, to prevent memory leak
    NET_DestroyDatagram( packet );
    
	packetsRead++;
	bytesRead += size;
	
	return true;
	// DG end
}


/*
========================
idUDP::GetPacketBlocking
========================
*/
bool idUDP::GetPacketBlocking( crAddress& from, void* data, size_t& size, size_t maxSize, int timeout )
{
#if USE_SDL3NET
	if ( !m_netSocket ) 
        return false;

	// Attempts to read immediately if a packet is already queued in SDL3 memory
	NET_Datagram *packet = nullptr;
    if ( NET_ReceiveDatagram( m_netSocket, &packet ) ) 
	{
        if ( packet ) 
		{
            size_t bytesCopied = ( packet->buflen > maxSize ) ? maxSize : packet->buflen;
            std::memcpy( data, packet->buf, bytesCopied );
            SDLAddressToIdAddress( packet->addr, packet->port, from );
            NET_DestroyDatagram( packet );
            size = bytesCopied;
        }
    }

    // If the internal queue was empty, we force an operating system block.
    // Since NET_WaitUntilInputAvailable accepts a generic array (void**), we create a single-element array.
	void *socketArray[1] = { (void*)m_netSocket };
    
	// If the timeout_ms parameter is less than 0 in the original call, we pass -1 (infinite wait)
    Sint32 sdlTimeout = ( timeout < 0 ) ? -1 : (Sint32)timeout;

	// Puts the engine thread to sleep until data is received or the timeout expires.
    // Returns > 0 if the socket has data available.
	int readySockets = NET_WaitUntilInputAvailable( socketArray, 1, sdlTimeout ); 
    if ( readySockets > 0 ) 
	{
		if ( NET_ReceiveDatagram( m_netSocket, &packet ) ) 
		{
            if ( packet ) 
			{
                size_t bytesCopied = ( packet->buflen > maxSize ) ? maxSize : packet->buflen;
                std::memcpy( data, packet->buf, bytesCopied );
                SDLAddressToIdAddress( packet->addr, packet->port, from );
                NET_DestroyDatagram( packet );
                size = bytesCopied;
            }
        }
    }

#else
	if( !crNetwork::Get()->WaitForData( netSocket, timeout ) )
		return false;
	
	if( GetPacket( from, data, size, maxSize ) )
		return true;
#endif
	return false;
}

/*
========================
idUDP::SendPacket
========================
*/
void idUDP::SendPacket( const netadr_t to, const void* data, size_t size )
{
	if( to.type == NA_BAD )
	{
		idLib::Warning( "idUDP::SendPacket: bad address type NA_BAD - ignored" );
		return;
	}
	
	packetsWritten++;
	bytesWritten += size;
	
	if( silent )
		return;

#if USE_SDL3NET
	uint16_t port;
	
	// Verify if our SDL3_net socket is active.
	if ( !m_netSocket )
		return;
	
	// Sends asynchronously. SDL places this in an internal queue and dispatches it.

	if ( !NET_SendDatagram( m_netSocket, to.address, port, data, size ) ) 
		idLib::Printf( "idUDP::SendPacket sendto error - packet dropped: %s\n", SDL_GetError() );

#else
	crNetwork::Get()->SendUDPPacket( netSocket, size, data, to );
#endif
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

bool crNetMessage::ReadBytes(char * bytes, int numBytes)
{
	assert(!(mDataOffset + numBytes > mDataSize));
	if (mDataOffset + numBytes > mDataSize)
		return false;

	char * ptr = mData + mDataOffset;
	mDataOffset += numBytes;
	std::memcpy(bytes, ptr, numBytes);
	return true;
}

bool crNetMessage::ReadString(char * buffer, int maxlength)
{
	unsigned short length = 0;
	return Read(length) && ReadBytes(buffer, length);
}

bool crNetMessage::WriteBytes(const char * bytes, int numBytes)
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

void crNetMessage::SendPacket(idUDP & socket, const netadr_t & addr)
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
crNetwork::crNetwork(void) : 
    m_numInterfaces( 0 )
{
}


void crNetwork::Init( void )
{
	// Try initialize SDL3_net
	if ( !NET_Init() ) 
    	idLib::Error(" Failed to initialize SDL3_net: %s", SDL_GetError()); 
	
	// Cria um socket UDP vinculado a qualquer interface na porta do Doom 3 (ex: 27666)
	m_gameSocket = NET_CreateDatagramSocket(NULL, 27666); //
	if (!m_gameSocket)
    	idLib::Error(" Failed to create net socket: %s", SDL_GetError());
}

void crNetwork::Shutdown( void )
{
}

int crNetwork::IPSocket(const char *bind_ip, int port, netadr_t *bound_to)
{
    return 0;
}

/*
========================
crNetwork::StringToNetAdr
========================
*/
bool crNetwork::StringToNetAdr( const char* s, netadr_t* a, bool doDNSResolve )
{
#if USE_SDL3NET
	int numBytes = 0;

	// Clear the destination structure to avoid memory garbage.
	a->type = NA_BAD;
	a->port = PORT_ANY;
	a->address = nullptr;
	std::memset( a->ip, 0x00, 16 );

	/// not valid 
	if ( !s || !s[0] )
        return false;

	// Handles the special "localhost" case that Doom 3 usually checks manually.
	idStr addressStr( s );
    if ( addressStr.Icmp( "localhost" ) == 0 ) 
        addressStr = "127.0.0.1";

	// If doDNSResolve is false, SDL3_net can still process the input if the string is already
	// a direct IP (e.g., "192.168.1.1").
    // SDL3_net resolves hostnames natively using controlled asynchronous or blocking methods.
    NET_Address* resolvedAddr = nullptr;

	// We attempt to resolve the address. We pass port 0 just to get the IP.
	if ( ( resolvedAddr = NET_ResolveHostname( addressStr.c_str() ) ) == nullptr ) 
	    return false;

	// Extract the formatted string from the IP resolved by SDL3_net
    // SDL3_net gives us the clean IP (whether converted from text or resolved via DNS)
	const char* ipString = NET_GetAddressString( resolvedAddr );
    if ( !ipString ) 
	{
        NET_UnrefAddress( resolvedAddr );
        return false;
    }

	// Populate the Doom 3 netadr_t structure based on the returned IP
    // Here, we parse the string returned by SDL into the netadr_t bytes.
    const void* rawBytes = NET_GetAddressBytes( resolvedAddr, &numBytes );
	
	// Identify the type and fill in the raw bytes
    if ( rawBytes && numBytes > 0 ) 
	{
        if ( numBytes == 16 ) 
		{
            // --- NATIVE IPv6 ALLOCATION ---
            a->type = NA_IP6;
            std::memcpy( a->ip, rawBytes, 16 );
        } 
        else if ( numBytes == 4 ) 
		{
			// --- NATIVE IPv4 ALLOCATION ---
            a->type = NA_IP;
            std::memcpy( a->ip, rawBytes, 4 ); 
		}
    }
	else
	{
		a->type = NA_BAD;
    }

	// Release SDL3_net address reference
	NET_UnrefAddress( resolvedAddr );

	return ( a->type != NA_BAD );
#else
	sockaddr_in sadr;
	if( !StringToSockaddr( s, &sadr, doDNSResolve ) )
		return false;
	
	SockadrToNetadr( &sadr, a );
#endif
	return true;
}

/*
========================
crNetwork::NetAdrToString
========================
*/
const char* crNetwork::NetAdrToString( const netadr_t a )
{
	// DG: FIXME: those static buffers look fishy - I would feel better if they were
	//            at least thread-local - so /maybe/ use ID_TLS here?
	//            or maybe return an idStr and change calling code accordingly
	ID_TLS int index = 0;
	ID_TLS char buf[ 4 ][ 64 ];	// flip/flop
	char* s;
	
	s = buf[index];
	index = ( index + 1 ) & 3;
	if( a.type == NA_IP || a.type == NA_LOOPBACK )
		idStr::snPrintf( s, 64, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], a.port );
	else if( a.type == NA_BROADCAST )
		idStr::snPrintf( s, 64, "BROADCAST" );
	else if( a.type == NA_BAD )
		idStr::snPrintf( s, 64, "BAD_IP" );
	else
		idStr::snPrintf( s, 64, "WTF_UNKNOWN_IP_TYPE_%i", a.type );
	
	return s;
}

/*
========================
crNetwork::IsLANAddress
========================
*/
bool crNetwork::IsLANAddress( const netadr_t adr )
{
	if( adr.type == NA_LOOPBACK )
		return true;
	
	if( adr.type != NA_IP )
		return false;
	
	// NOTE: this function won't work reliably for addresses on the local net
	// that are connected through a router (i.e. no IP from that net is on any interface)
	// However, I don't expect most people to have such setups at home and the code
	// would get a lot more complex and less portable.
	// Furthermore, this function isn't even used currently
	if( m_numInterfaces )
	{
		int i;
		// DG: for 64bit compatibility, make these longs ints.
		uint32_t* p_ip;
		uint32_t ip;
		p_ip = ( uint32_t* )&adr.ip[0];
		// DG end
		ip = ntohl( *p_ip );
		
		for( i = 0; i < m_numInterfaces; i++ )
		{
			if( ( m_netint[i].ip & m_netint[i].mask ) == ( ip & m_netint[i].mask ) )
				return true;
		}
	}

	return false;
}

/*
========================
crNetwork::CompareNetAdrBase

Compares without the port.
========================
*/
bool crNetwork::CompareNetAdrBase( const netadr_t a, const netadr_t b ) const
{
#if USE_SDL3NET
	// TODO: store andresses 
	if( NET_CompareAddresses( a.address, b.address ) == 0 )
		return true;
#else

	if( a.type != b.type )
		return false;
	
	if( a.type == NA_LOOPBACK )
	{
		// DG: wtf is this comparison about, the comment above says "without the port"
		if( a.port == b.port )
			return true;
		
		return false;
	}
	
	if( a.type == NA_IP )
	{
		if( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] )
			return true;

		return false;
	}	
#endif

	idLib::Printf( "CompareNetAdrBase: bad address type\n" );
	return false;
}

/*
========================
crNetwork::GetLocalIPCount
========================
*/
uint32_t crNetwork::GetLocalIPCount( void ) const
{
	return m_numInterfaces;
}

/*
========================
crNetwork::GetLocalIP
========================
*/
const char* crNetwork::GetLocalIP( int i ) const
{
	if( ( i < 0 ) || ( i >= m_numInterfaces ) )
		return nullptr;
	
	return m_netint[i].addr;
}


/*
========================
crNetwork::ExtractPort
========================
*/
bool crNetwork::ExtractPort( const char* src, char* buf, const size_t bufsize, int* port )
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

/*
========================
crNetwork::StringToSockaddr
========================
*/
bool crNetwork::StringToSockaddr( const char* s, sockaddr_in* sadr, const bool doDNSResolve )
{
#if USE_SDL3NET
	// Clear the destination structure to avoid memory garbage.
	sadr->sin_addr = nullptr;
	sadr->sin_family =

#else
	char buf[256];
	int port;
	
	std::memset( sadr, 0, sizeof( *sadr ) );
	
	sadr->sin_family = AF_INET;
	sadr->sin_port = 0;
	
	// try to remove the port first, otherwise the DNS gets confused into multiple timeouts
	// failed or not failed, buf is expected to contain the appropriate host to resolve
	if( ExtractPort( s, buf, sizeof( buf ), &port ) )
		sadr->sin_port = htons( port );

#if 0 // DEPRECATED 
	// NOTE: the doDNSResolve argument is ignored for two reasons:
	// 1. domains can start with numbers nowadays so the old heuristic to find out if it's
	//    an IP (check if the first char is a digit) isn't reliable
	// 2. gethostbyname() works fine for IPs and doesn't do a lookup if the passed string
	//    is an IP

    // buf contains the host, even if Net_ExtractPort returned false
	struct hostent*	h;
    h = gethostbyname( buf );
	if( h == nullptr )
		return false;
    
    sadr->sin_addr.s_addr = *( in_addr_t* ) h->h_addr_list[0];	
#else
    // do the DNS resolution
    struct addrinfo hints, *res = nullptr;
    int status = getaddrinfo( buf, nullptr, &hints, &res);
    if (status != 0)
        return false;

    // Copies the resolved IPv4 address to the destination structure
    struct sockaddr_in* ipv4 = ( struct sockaddr_in* )res->ai_addr;
	sadr->sin_addr = ipv4->sin_addr;
	
	// Frees the memory dynamically allocated by getaddrinfo
    freeaddrinfo( res );
#endif
#endif
	return true;
}

/*
========================
crNetwork::SockadrToNetadr
========================
*/
void crNetwork::SockadrToNetadr( sockaddr_in* s, netadr_t* a )
{
	in_addr_t ip;
	if( s->sin_family == AF_INET )
	{
		ip = s->sin_addr.s_addr;
		*( in_addr_t* )&a->ip = ip;
		a->port = ntohs( s->sin_port );
		// we store in network order, that loopback test is host order..
		ip = ntohl( ip );
		// DG: just comparing ip with INADDR_LOOPBACK is lame,
		//     because all of 127.0.0.0/8 is loopback.
		// if( ( ip & LOOPBACK_PREFIX ) == LOOPBACK_NET )
		if( ip == INADDR_LOOPBACK )
			a->type = NA_LOOPBACK;
		else
			a->type = NA_IP;
	}
}

/*
========================
crNetwork::NetadrToSockadr
========================
*/
void crNetwork::NetadrToSockadr( const netadr_t* a, sockaddr_in* s )
{
	std::memset( s, 0, sizeof( *s ) );
	
	if( a->type == NA_BROADCAST )
	{
		s->sin_family = AF_INET;
		s->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if( a->type == NA_IP || a->type == NA_LOOPBACK )
	{
		s->sin_family = AF_INET;
		s->sin_addr.s_addr = *( ( in_addr_t* ) &a->ip );
	}
	
	s->sin_port = htons( ( short )a->port );
}
