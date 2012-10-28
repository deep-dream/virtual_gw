/*=============================================================================
    "HeadLong Library"
    ______________________

    socket.h:	(created 2001/02/02 	0:03)
  
        Description:
			
	
	  
        Copyright (c) Artyom V. Borodkin, 2001.  All Rights Reserved.
                                                   (http://www.deep-dream.com)
=============================================================================*/
#pragma once
#include <vector>
#include "address.h"
namespace net{

// Maximum queue length specifiable by listen.	
#define HL_SOMAXCONN       0x7fffffff
#define HL_SD_RECEIVE      0x00
#define HL_SD_SEND         0x01
#define HL_SD_BOTH         0x02
	
class socket{
	dword timeout;
	dword last_transfer;
	dword	s;// SOCKET
	
	socket(const socket&);
	socket& operator=(socket&);
public:
	socket(const dword ); // SOCKET	

	// AF_INET, SOCK_DGRAM, 0
	socket(int af,int type,int protocol);
	
	virtual	int	is_timeout();
	virtual	void	set_timeout(dword timeout);
	virtual	int	bind(address& addr);
	virtual	int	listen(int backlog = HL_SOMAXCONN );

	virtual	dword accept(address& addr = address());

	virtual	address	getaddress();
	virtual	address	getpeeraddress();

	virtual	int	connect(address& addr);
	virtual	int	send(const std::vector< char >& data);
	virtual	int	sendto(address& addr,const std::vector< char >& data);

	virtual	int	receive(std::vector< char >& data, int flags = NULL );
	virtual	int	receivefrom(address& addr,std::vector< char >& data, int flags = NULL);

	virtual	int	canread(int timeout = 0); //milliseconds
	virtual	int	canwrite(int timeout = 0);//milliseconds
	
	virtual	int	getopt(int level,int optname);	
	virtual	int	setopt(int level,int optname,int value);
	virtual	int	setopt(int level,int optname,const char* value, int v_len);

	virtual	int shutdown( int how );
	virtual	int close( );
	virtual	int graceful_shutdown ( );

	virtual	~socket();
};
}