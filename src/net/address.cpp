#include "net/address.h"
#include "net/net.h"
#include <winsock.h>
//#include <windows.h>

using	namespace	net;

address::address(std::string host,word port)
{
	_port = port;
	unsigned long res = inet_addr( host.c_str() );
	if(res != INADDR_NONE)
		_host = ntohl( res );
	else
		throw net_exception(WSAGetLastError());
}

std::string address::IPAsString()const
{
	unsigned char  c_host[4];
	*(dword*)c_host = host();
	char	buffer[256];
	sprintf( buffer, "%d.%d.%d.%d",int(c_host[0]),int(c_host[1]),int(c_host[2]),int(c_host[3]) );
	return buffer;
}

dword	address::host()const
{	
	return _host;
}

word address::port()const
{
	return _port;
}