
#include "precompiled.h"
#include "Windows_Network.hpp"

typedef ULONG in_addr_t;
typedef int socklen_t;

/*
========================
crWindowsNetwork::IPSocket
========================
*/
int crWindowsNetwork::IPSocket( const char* bind_ip, int port, netadr_t* bound_to )
{
	SOCKET				newsocket;
	sockaddr_in			address;
	
	if( port != PORT_ANY )
	{
		if( bind_ip )
			idLib::Printf( "Opening IP socket: %s:%i\n", bind_ip, port );
		else
			idLib::Printf( "Opening IP socket: localhost:%i\n", port );
	}
	
	if( ( newsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: socket: %s\n", ErrorString() );
		return 0;
	}
	
	// make it non-blocking
	unsigned long	_true = 1;
	if( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", ErrorString() );
		closesocket( newsocket );
		return 0;
	}
	
	// make it broadcast capable
	int i = 1;
	if( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, ( char* )&i, sizeof( i ) ) == SOCKET_ERROR )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", ErrorString() );
		closesocket( newsocket );
		return 0;
	}
	
	if( !bind_ip || !bind_ip[0] || !idStr::Icmp( bind_ip, "localhost" ) )
		address.sin_addr.s_addr = INADDR_ANY;
	else
		StringToSockaddr( bind_ip, &address, true );
	
	if( port == PORT_ANY )
		address.sin_port = 0;
	else
		address.sin_port = htons( ( short )port );
	
	address.sin_family = AF_INET;
	
	if( bind( newsocket, ( const sockaddr* )&address, sizeof( address ) ) == SOCKET_ERROR )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: bind: %s\n", ErrorString() );
		closesocket( newsocket );
		return 0;
	}
	
	// if the port was PORT_ANY, we need to query again to know the real port we got bound to
	// ( this used to be in idUDP::InitForPort )
	if( bound_to )
	{
		socklen_t len = sizeof( address );
		if( getsockname( newsocket, ( struct sockaddr* )&address, &len ) == SOCKET_ERROR )
		{
			common->Printf( "ERROR: IPSocket: getsockname: %s\n", ErrorString() );
			closesocket( newsocket );
			return 0;
		}

		SockadrToNetadr( &address, bound_to );
	}
	
	return newsocket;
}

/*
========================
crWindowsNetwork::GetUDPPacket
========================
*/
bool crWindowsNetwork::GetUDPPacket( int netSocket, netadr_t& net_from, char* data, size_t& size, size_t maxSize )
{
	int 			ret;
	sockaddr_in		from;
	socklen_t		fromlen;
	int				err;
	
	if( !netSocket )
		return false;
	
	fromlen = sizeof( from );
	ret = recvfrom( netSocket, data, maxSize, 0, ( sockaddr* )&from, &fromlen );
	if( ret == SOCKET_ERROR )
	{
		err = WSAGetLastError();
		
		if( err == WSAEWOULDBLOCK || err == WSAECONNRESET )
			return false;
		
		idLib::Printf( "GetUDPPacket: %s\n", ErrorString() );
		return false;
	}
#if 0 // TODO: WTF was this about?
	// DG: ip_socket is never initialized, so this is dead code
	// - and if netSocket is 0 (so this would be true) recvfrom above will already fail
	if( static_cast<unsigned int>( netSocket ) == ip_socket )
	{
		std::memset( from.sin_zero, 0, sizeof( from.sin_zero ) );
	}
	
	if( usingSocks && static_cast<unsigned int>( netSocket ) == ip_socket && memcmp( &from, &socksRelayAddr, fromlen ) == 0 )
	{
		if( ret < 10 || data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 1 )
		{
			return false;
		}
		net_from.type = NA_IP;
		net_from.ip[0] = data[4];
		net_from.ip[1] = data[5];
		net_from.ip[2] = data[6];
		net_from.ip[3] = data[7];
		net_from.port = *( short* )&data[8];
		memmove( data, &data[10], ret - 10 );
	}
	else
	{
#endif // 0
		SockadrToNetadr( &from, &net_from );
#if 0 // this is ugly, but else astyle is confused
	}
#endif
	
	if( ret > maxSize )
	{
		idLib::Printf( "GetUDPPacket: oversize packet from %s\n", NetAdrToString( net_from ) );
		return false;
	}
	
	size = ret;
	
	return true;
}

/*
========================
crWindowsNetwork::WaitForData
========================
*/
bool crWindowsNetwork::WaitForData( int netSocket, int timeout )
{
	int					ret;
	fd_set				set;
	struct timeval		tv;
	
	if( !netSocket )
		return false;
	
	if( timeout < 0 )
		return true;
	
	FD_ZERO( &set );
	FD_SET( netSocket, &set ); // TODO: winsocks may want an unsigned int for netSocket?
	
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = ( timeout % 1000 ) * 1000;
	
	ret = select( netSocket + 1, &set, nullptr, nullptr, &tv );
	
	if( ret == -1 )
	{
		idLib::Printf( "WaitForData select(): %s\n", ErrorString() );
		return false;
	}
	
	// timeout with no data
	if( ret == 0 )
		return false;
	
	return true;
}

/*
========================
crWindowsNetwork::SendUDPPacket
========================
*/
void crWindowsNetwork::SendUDPPacket( int netSocket, size_t length, const void* data, const netadr_t to )
{
	int				ret;
	sockaddr_in		addr;
	
	if( !netSocket )
		return;
	
	NetadrToSockadr( &to, &addr );
	
	if( m_usingSocks && to.type == NA_IP )
	{
		m_socksBuf[0] = 0;	// reserved
		m_socksBuf[1] = 0;
		m_socksBuf[2] = 0;	// fragment (not fragmented)
		m_socksBuf[3] = 1;	// address type: IPV4
		*( int* )&m_socksBuf[4] = addr.sin_addr.s_addr;
		*( short* )&m_socksBuf[8] = addr.sin_port;
		std::memcpy( &m_socksBuf[10], data, length );
		ret = sendto( netSocket, m_socksBuf, length + 10, 0, ( sockaddr* )&m_socksRelayAddr, sizeof( m_socksRelayAddr ) );
	}
	else
	{
		ret = sendto( netSocket, ( const char* )data, length, 0, ( sockaddr* )&addr, sizeof( addr ) );
	}

	if( ret == SOCKET_ERROR )
	{
		int err = WSAGetLastError();
		// some PPP links do not allow broadcasts and return an error
		if( ( err == WSAEADDRNOTAVAIL ) && ( to.type == NA_BROADCAST ) )
			return;
		
		// NOTE: EWOULDBLOCK used to be silently ignored,
		// but that means the packet will be dropped so I don't feel it's a good thing to ignore
		idLib::Printf( "UDP sendto error - packet dropped: %s\n", ErrorString() );
	}
}

/*
========================
crWindowsNetwork::ErrorString
========================
*/
const char* crWindowsNetwork::ErrorString( void ) const
{
	int code = WSAGetLastError();
	switch( code )
	{
		case WSAEINTR:
			return "WSAEINTR";
		case WSAEBADF:
			return "WSAEBADF";
		case WSAEACCES:
			return "WSAEACCES";
		case WSAEDISCON:
			return "WSAEDISCON";
		case WSAEFAULT:
			return "WSAEFAULT";
		case WSAEINVAL:
			return "WSAEINVAL";
		case WSAEMFILE:
			return "WSAEMFILE";
		case WSAEWOULDBLOCK:
			return "WSAEWOULDBLOCK";
		case WSAEINPROGRESS:
			return "WSAEINPROGRESS";
		case WSAEALREADY:
			return "WSAEALREADY";
		case WSAENOTSOCK:
			return "WSAENOTSOCK";
		case WSAEDESTADDRREQ:
			return "WSAEDESTADDRREQ";
		case WSAEMSGSIZE:
			return "WSAEMSGSIZE";
		case WSAEPROTOTYPE:
			return "WSAEPROTOTYPE";
		case WSAENOPROTOOPT:
			return "WSAENOPROTOOPT";
		case WSAEPROTONOSUPPORT:
			return "WSAEPROTONOSUPPORT";
		case WSAESOCKTNOSUPPORT:
			return "WSAESOCKTNOSUPPORT";
		case WSAEOPNOTSUPP:
			return "WSAEOPNOTSUPP";
		case WSAEPFNOSUPPORT:
			return "WSAEPFNOSUPPORT";
		case WSAEAFNOSUPPORT:
			return "WSAEAFNOSUPPORT";
		case WSAEADDRINUSE:
			return "WSAEADDRINUSE";
		case WSAEADDRNOTAVAIL:
			return "WSAEADDRNOTAVAIL";
		case WSAENETDOWN:
			return "WSAENETDOWN";
		case WSAENETUNREACH:
			return "WSAENETUNREACH";
		case WSAENETRESET:
			return "WSAENETRESET";
		case WSAECONNABORTED:
			return "WSAECONNABORTED";
		case WSAECONNRESET:
			return "WSAECONNRESET";
		case WSAENOBUFS:
			return "WSAENOBUFS";
		case WSAEISCONN:
			return "WSAEISCONN";
		case WSAENOTCONN:
			return "WSAENOTCONN";
		case WSAESHUTDOWN:
			return "WSAESHUTDOWN";
		case WSAETOOMANYREFS:
			return "WSAETOOMANYREFS";
		case WSAETIMEDOUT:
			return "WSAETIMEDOUT";
		case WSAECONNREFUSED:
			return "WSAECONNREFUSED";
		case WSAELOOP:
			return "WSAELOOP";
		case WSAENAMETOOLONG:
			return "WSAENAMETOOLONG";
		case WSAEHOSTDOWN:
			return "WSAEHOSTDOWN";
		case WSASYSNOTREADY:
			return "WSASYSNOTREADY";
		case WSAVERNOTSUPPORTED:
			return "WSAVERNOTSUPPORTED";
		case WSANOTINITIALISED:
			return "WSANOTINITIALISED";
		case WSAHOST_NOT_FOUND:
			return "WSAHOST_NOT_FOUND";
		case WSATRY_AGAIN:
			return "WSATRY_AGAIN";
		case WSANO_RECOVERY:
			return "WSANO_RECOVERY";
		case WSANO_DATA:
			return "WSANO_DATA";
		default:
			return "NO ERROR";
	}
}
