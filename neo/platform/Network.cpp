
#include "precompiled.h"
#include "Network.hpp"

#if __PLATFORM_LINUX__
#   include <arpa/inet.h>
#   include <netdb.h>
#else if __PLATFORM_WINDOWS__
#   include <winsock2.h>
#   include <ws2tcpip.h>
#endif

idCVar crNetwork::net_socksServer( "net_socksServer", "", CVAR_ARCHIVE, "" );
idCVar crNetwork::net_socksPort( "net_socksPort", "1080", CVAR_ARCHIVE | CVAR_INTEGER, "" );
idCVar crNetwork::net_socksUsername( "net_socksUsername", "", CVAR_ARCHIVE, "" );
idCVar crNetwork::net_socksPassword( "net_socksPassword", "", CVAR_ARCHIVE, "" );
idCVar crNetwork::net_ip( "net_ip", "localhost", CVAR_NOCHEAT, "local IP address" );

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
	netSocket = 0;
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
	
	return true;
}

/*
========================
idUDP::Close
========================
*/
void idUDP::Close( void )
{
	if( netSocket )
	{
#if __PLATFORM_LINUX__
        close( netSocket );
#else
		closesocket( netSocket );
#endif
		netSocket = 0;
		std::memset( &bound_to, 0, sizeof( bound_to ) );
	}
}

/*
========================
idUDP::GetPacket
========================
*/
bool idUDP::GetPacket( netadr_t& from, void* data, size_t& size, size_t maxSize )
{
	// DG: this fake while(1) loop pissed me off so I replaced it.. no functional change.
	if( ! crNetwork::Get()->GetUDPPacket( netSocket, from, ( char* )data, size, maxSize ) )
		return false;
	
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
bool idUDP::GetPacketBlocking( netadr_t& from, void* data, size_t& size, size_t maxSize, int timeout )
{
	if( !crNetwork::Get()->WaitForData( netSocket, timeout ) )
		return false;
	
	if( GetPacket( from, data, size, maxSize ) )
		return true;
	
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
	
	crNetwork::Get()->SendUDPPacket( netSocket, size, data, to );
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

bool crNetMessage::ReadPacket(idUDP & socket, netadr_t & addrFrom)
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

/*
========================
crNetwork::StringToNetAdr
========================
*/
bool crNetwork::StringToNetAdr( const char* s, netadr_t* a, bool doDNSResolve )
{
	sockaddr_in sadr;
	
	if( !StringToSockaddr( s, &sadr, doDNSResolve ) )
		return false;
	
	SockadrToNetadr( &sadr, a );
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
	
	idLib::Printf( "Sys_CompareNetAdrBase: bad address type\n" );
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
