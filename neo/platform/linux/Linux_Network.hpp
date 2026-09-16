
#ifndef __LINUX_NETWORK_HPP__
#define __LINUX_NETWORK_HPP__

typedef int SOCKET;

class crLinuxNetwork : public crNetwork
{
public:
    crLinuxNetwork( void );
    ~crLinuxNetwork( void );

    virtual void			Init( void );
    virtual void			Shutdown( void );

    virtual int				IPSocket( const char* bind_ip, int port, netadr_t* bound_to );
	virtual bool			GetUDPPacket( int netSocket, netadr_t& net_from, char* data, size_t& size, size_t maxSize );
	virtual bool			WaitForData( int netSocket, int timeout );
	virtual void			SendUDPPacket( int netSocket, size_t length, const void* data, const netadr_t to );

protected:
	virtual const char*     ErrorString( void );

private:
    bool    m_usingSocks;
    SOCKET  m_socksSocket;
};

#endif //!__LINUX_NETWORK_HPP__