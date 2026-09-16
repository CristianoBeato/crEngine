
#include "precompiled.h"
#include "Linux_Network.hpp"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <errno.h>
#include <sys/select.h>
#include <net/if.h>


#define D3_NET_EWOULDBLOCK   EWOULDBLOCK
#define D3_NET_ECONNRESET    ECONNRESET
#define D3_NET_EADDRNOTAVAIL EADDRNOTAVAIL

constexpr int SOCKET_ERROR = -1;
constexpr int INVALID_SOCKET = -1;

crNetwork* crNetwork::Get( void ) 
{
    static crLinuxNetwork gLinuxNetwork = crLinuxNetwork();
    return &gLinuxNetwork;
}

crLinuxNetwork::crLinuxNetwork( void ) : crNetwork()
{
}

crLinuxNetwork::~crLinuxNetwork( void )
{
}

/*
========================
NET_ErrorString
========================
*/
const char* crLinuxNetwork::ErrorString( void ) const
{
    return strerror( errno );
}

// TODO: Lambda ? 
static void ip_to_addr( const char ip[4], char* addr )
{
	idStr::snPrintf( addr, 16, "%d.%d.%d.%d", ( unsigned char )ip[0], ( unsigned char )ip[1], ( unsigned char )ip[2], ( unsigned char )ip[3] );
}

void crLinuxNetwork::Init(void)
{
    bool foundloopback = false;

    int		s;
	char	buf[ MAX_INTERFACES * sizeof( ifreq ) ];
	ifconf	ifc;
	ifreq*	ifr;
	int		ifindex;
	unsigned int ip, mask;
	
	m_numInterfaces = 0;
	
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	ifc.ifc_len = MAX_INTERFACES * sizeof( ifreq );
	ifc.ifc_buf = buf;
	if( ioctl( s, SIOCGIFCONF, &ifc ) < 0 )
	{
		common->FatalError( "InitNetworking: SIOCGIFCONF error - %s\n", strerror( errno ) );
		return;
	}
	ifindex = 0;
	while( ifindex < ifc.ifc_len )
	{
		common->Printf( "found interface %s - ", ifc.ifc_buf + ifindex );
		// find the type - ignore interfaces for which we can find we can't get IP and mask ( not configured )
		ifr = ( ifreq* )( ifc.ifc_buf + ifindex );
		if( ioctl( s, SIOCGIFADDR, ifr ) < 0 )
		{
			common->Printf( "SIOCGIFADDR failed: %s\n", strerror( errno ) );
		}
		else
		{
			if( ifr->ifr_addr.sa_family != AF_INET )
			{
				common->Printf( "not AF_INET\n" );
			}
			else
			{
				// RB: 64 bit fixes, changed long to int
				ip = ntohl( *( unsigned int* )&ifr->ifr_addr.sa_data[2] );
				// RB end
				if( ip == INADDR_LOOPBACK )
				{
					foundloopback = true;
					common->Printf( "loopback\n" );
				}
				else
				{
					common->Printf( "%d.%d.%d.%d",
									( unsigned char )ifr->ifr_addr.sa_data[2],
									( unsigned char )ifr->ifr_addr.sa_data[3],
									( unsigned char )ifr->ifr_addr.sa_data[4],
									( unsigned char )ifr->ifr_addr.sa_data[5] );
				}
	
				// DG: set netint address before getting the mask
				ip_to_addr( &ifr->ifr_addr.sa_data[2], m_netint[ m_numInterfaces ].addr );
				// DG end
	
				if( ioctl( s, SIOCGIFNETMASK, ifr ) < 0 )
				{
					common->Printf( " SIOCGIFNETMASK failed: %s\n", strerror( errno ) );
				}
				else
				{
					// RB: 64 bit fixes, changed long to int
					mask = ntohl( *( unsigned int* )&ifr->ifr_addr.sa_data[2] );
					// RB end
					if( ip != INADDR_LOOPBACK )
					{
						common->Printf( "/%d.%d.%d.%d\n",
										( unsigned char )ifr->ifr_addr.sa_data[2],
										( unsigned char )ifr->ifr_addr.sa_data[3],
										( unsigned char )ifr->ifr_addr.sa_data[4],
										( unsigned char )ifr->ifr_addr.sa_data[5] );
					}

					m_netint[ m_numInterfaces ].ip = ip;
					m_netint[ m_numInterfaces ].mask = mask;
					m_numInterfaces++;
				}
			}
		}
		ifindex += sizeof( ifreq );
	}

    // for some retarded reason, win32 doesn't count loopback as an adapter...
	// and because I'm extra-cautious I add this check on real operating systems as well :)
	if( !foundloopback && m_numInterfaces < MAX_INTERFACES )
	{
		idLib::Printf( "Sys_InitNetworking: adding loopback interface\n" );
		m_netint[m_numInterfaces].ip = ntohl( inet_addr( "127.0.0.1" ) );
		m_netint[m_numInterfaces].mask = ntohl( inet_addr( "255.0.0.0" ) );
		m_numInterfaces++;
	}
}

void crLinuxNetwork::Shutdown(void)
{
    if( m_usingSocks )
		close( m_socksSocket );
}

/*
========================
crLinuxNetwork::IPSocket
========================
*/
int crLinuxNetwork::IPSocket( const char* bind_ip, int port, netadr_t* bound_to )
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
	int flags = fcntl( newsocket, F_GETFL, 0 );
	if( flags < 0 )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: fcntl F_GETFL: %s\n", ErrorString() );
		close( newsocket );
		return 0;
	}

	flags |= O_NONBLOCK;
	if( fcntl( newsocket, F_SETFL, flags ) < 0 )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: fcntl F_SETFL with O_NONBLOCK: %s\n", ErrorString() );
		close( newsocket );
		return 0;
	}
	
	// make it broadcast capable
	int i = 1;
	if( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, ( char* )&i, sizeof( i ) ) == SOCKET_ERROR )
	{
		idLib::Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", ErrorString() );
		close( newsocket );
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
		close( newsocket );
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
			close( newsocket );
			return 0;
		}

		SockadrToNetadr( &address, bound_to );
	}
	
	return newsocket;
}


/*
========================
crLinuxNetwork::GetUDPPacket
========================
*/
bool crLinuxNetwork::GetUDPPacket( int netSocket, netadr_t& net_from, char* data, size_t& size, size_t maxSize )
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
		err = errno;//GetLastError();
		
		if( err == D3_NET_EWOULDBLOCK || err == D3_NET_ECONNRESET )
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
		idLib::Printf( "Net_GetUDPPacket: oversize packet from %s\n", NetAdrToString( net_from ) );
		return false;
	}
	
	size = ret;
	
	return true;
}

/*
========================
crLinuxNetwork::WaitForData
========================
*/
bool crLinuxNetwork::WaitForData( int netSocket, int timeout )
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
crLinuxNetwork::SendUDPPacket
========================
*/
void crLinuxNetwork::SendUDPPacket( int netSocket, size_t length, const void* data, const netadr_t to )
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
		int err = errno;
		// some PPP links do not allow broadcasts and return an error
		if( ( err == D3_NET_EADDRNOTAVAIL ) && ( to.type == NA_BROADCAST ) )
			return;
		
		// NOTE: EWOULDBLOCK used to be silently ignored,
		// but that means the packet will be dropped so I don't feel it's a good thing to ignore
		idLib::Printf( "UDP sendto error - packet dropped: %s\n", ErrorString() );
	}
}
