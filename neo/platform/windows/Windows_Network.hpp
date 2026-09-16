
#ifndef __WINDOWS_NETWORK_HPP__
#define __WINDOWS_NETWORK_HPP__

class crWindowsNetwork : public crNetwork
{
public:
    crWindowsNetwork( void );
    ~crWindowsNetwork( void );

    virtual void			Init( void );
    virtual void			Shutdown( void );

    virtual int				IPSocket( const char* bind_ip, int port, netadr_t* bound_to );
	virtual bool			GetUDPPacket( int netSocket, netadr_t& net_from, char* data, size_t& size, size_t maxSize );
	virtual bool			WaitForData( int netSocket, int timeout );
	virtual void			SendUDPPacket( int netSocket, size_t length, const void* data, const netadr_t to );

protected:
	virtual const char*     ErrorString( void ) const;

private:
    bool    m_usingSocks;
    bool	m_winsockInitialized = false;
    WSADATA	m_winsockdata;
    SOCKET  m_socksSocket;
};

#endif //!__WINDOWS_NETWORK_HPP__   