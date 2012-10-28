#include <tchar.h>
#include <winsock2.h>
#include "net/socket.h"
#include "net/net.h"

#define SO_MAX_MSG_SIZE   0x2003      /* maximum message size */

using	namespace	net;

namespace{
	int socket_counter = 0;
};

void	StartWinSock()
{
	socket_counter++;
	if( socket_counter != 1 )return;

	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;	
	
	if (WSAStartup( wVersionRequested, &wsaData )) {
		MessageBox(NULL,_T("WSAStartup failed"),_T("StartWinSock"),MB_OK);
		return;
	}	
	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
		MessageBox(NULL,_T("WinSock wrong version"),_T("StartWinSock"),MB_OK);
		WSACleanup();
		return; 
	}	
}

void	StoptWinSock()
{
	socket_counter--;
	if( socket_counter != 0 )return;

	if(int res = WSACleanup()){
		MessageBox(NULL,_T("WSACleanup() failed"),_T("StoptWinSock"),MB_OK);
	}
}

socket::socket(const dword new_s ):timeout(0)
{
	guard("socket::socket(const dword new_s )")
	StartWinSock();
	s = new_s;
	last_transfer = GetTickCount();
	unguard
}

socket::socket(int af,int type,int protocol):timeout(0)
{
	guard("socket::socket")
	StartWinSock();
	
	s = ::socket(af, type, protocol);
	if ( s == INVALID_SOCKET ) throw net_exception(WSAGetLastError());
	last_transfer = GetTickCount();
	unguard
}

int	socket::listen(int backlog)
{
	guard("socket::listen")
		
	if( ::listen( s, backlog  ) == SOCKET_ERROR )
		throw net_exception(WSAGetLastError());
	
	return 0;
	unguard
	return 0;
}

dword	socket::accept( address& addr )
{
	guard("socket::accept")

	struct sockaddr_in in_addr;
	int addrlen =  sizeof( in_addr );

	SOCKET new_s = ::accept(s, (LPSOCKADDR)&in_addr, &addrlen );
	if( new_s  == INVALID_SOCKET )
		throw net_exception(WSAGetLastError());		

	addr = net::address( in_addr.sin_addr.s_addr, in_addr.sin_port );

	return new_s;
	unguard
	return 0;
}

int	socket::bind(address& addr)
{
	guard("socket::bind")
	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET; // should be the same as has constructor
    addr_in.sin_port = htons( addr.port() );
	addr_in.sin_addr.s_addr = htonl( addr.host() );

	if(::bind(s, (LPSOCKADDR)&addr_in, sizeof(addr_in)) == INVALID_SOCKET)
		throw net_exception(WSAGetLastError());

	return 0;
	unguard
}

address	socket::getaddress()
{
	guard("socket::getaddress")
	struct sockaddr_in addr_in;
	int length = sizeof(addr_in);

	if(getsockname(s,(LPSOCKADDR)&addr_in,&length )==SOCKET_ERROR)
		throw net_exception(WSAGetLastError());
	
	return address(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));
	unguard
}

address	socket::getpeeraddress()
{
	guard("socket::getpeeraddress")
	struct sockaddr_in addr_in;
	int length = sizeof(addr_in);	

	if(getpeername(s,(LPSOCKADDR)&addr_in,&length))
		throw net_exception(WSAGetLastError());
	return address(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));
	unguard
}

int	socket::connect(address& addr)
{
	guard("socket::connect")
	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET; // should be the same as has constructor
	addr_in.sin_port = htons( addr.port() );
	addr_in.sin_addr.s_addr = htonl( addr.host() );

	int result = ::connect( s, (LPSOCKADDR)&addr_in, sizeof(addr_in) );
	if( result == SOCKET_ERROR)
		throw net_exception(WSAGetLastError());

	return result;
	unguard
	return 0;
}

int	socket::send(const std::vector< char >& data)
{
	guard("socket::send")
	int result = ::send(s, &data.front(), int(data.size()), MSG_DONTROUTE );
	if( result == SOCKET_ERROR)
		throw net_exception(WSAGetLastError());
	last_transfer = GetTickCount();
	return result;
	unguard
	return 0;
}

int	socket::sendto(address& addr,const std::vector< char >& data)
{
	guard("socket::sendto")
	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET; // should be the same as has constructor
	addr_in.sin_port = htons( addr.port() );
	addr_in.sin_addr.s_addr = htonl( addr.host() );

	int	sent_number = ::sendto(s,&data.front(),int(data.size()),NULL,(LPSOCKADDR)&addr_in, sizeof(addr_in));
	if(sent_number == SOCKET_ERROR)
		throw net_exception(WSAGetLastError());
	last_transfer = GetTickCount();
	return sent_number;
	unguard
}

int	socket::receive(std::vector< char >& data, int flags)
{
printf("socket::receive\n");
	guard("socket::receive")
	//int length = getopt(SOL_SOCKET,SO_MAX_MSG_SIZE);	
	u_long length=1024;

	if( ::ioctlsocket( s, FIONREAD, &length ) == SOCKET_ERROR )
		throw net_exception(WSAGetLastError());
//------------------------------------

	WSAEVENT Event;
	WSAOVERLAPPED AcceptOverlapped;
	WSABUF DataBuf;
	DWORD 	RecvBytes = 0, 
		Flags = 0, 
		BytesTransferred = 0;

	Event = WSACreateEvent();
	ZeroMemory(&AcceptOverlapped, sizeof(WSAOVERLAPPED));
	AcceptOverlapped.hEvent = Event;
	data.resize(SO_MAX_MSG_SIZE);
	DataBuf.len = SO_MAX_MSG_SIZE;
	DataBuf.buf = &data[0];

	if (WSARecv(s, &DataBuf, 1, &RecvBytes, &Flags, &AcceptOverlapped, NULL) == SOCKET_ERROR)
	{
    	if (WSAGetLastError() == WSA_IO_PENDING)
		{
			DWORD to = (timeout) ? timeout : WSA_INFINITE;
			WSAWaitForMultipleEvents(1, &Event, FALSE, to, FALSE);
			WSAResetEvent(Event);
			WSAGetOverlappedResult(s, &AcceptOverlapped, &BytesTransferred, FALSE, &Flags);
			WSACloseEvent(Event);
			data.resize(BytesTransferred);
			if (BytesTransferred)
				last_transfer = GetTickCount();
		}
		else
		{
			WSACloseEvent(Event);
			data.resize(0);
			throw net_exception(WSAGetLastError());
		}
	}
	else
	{
		if (RecvBytes)
			last_transfer = GetTickCount();
		WSACloseEvent(Event);
		data.resize(RecvBytes);
	}

//----------------------------------
/*
	// if length == 0
	// This could be a close conncetion case
	// when we should read 0 bytes.	
	if(length==0) length=1024;

	char*	buffer = new char[ length ];
	int res = recv( s, buffer, length, flags );
	if( res == SOCKET_ERROR){
		delete buffer;
		throw net_exception(WSAGetLastError());
	}
	last_transfer = GetTickCount();
	data.resize(res);
	if(0!=res)
		memcpy(&data.front(),buffer,res);
	delete	buffer;
*/
	return int(data.size());	
	unguard
}

int	socket::receivefrom(address& addr,std::vector< char >& data, int flags)
{
	guard("socket::receivefrom")
	int length = getopt(SOL_SOCKET,SO_MAX_MSG_SIZE);	
	char*	buffer = new char[length];

	sockaddr_in addr_from;
	int	addr_from_size = sizeof(addr_from);

	int res = recvfrom(s,buffer,length,flags,(sockaddr*)&addr_from,&addr_from_size);
	if( res == SOCKET_ERROR){
		delete buffer;
		throw net_exception(WSAGetLastError());				
	}
	last_transfer = GetTickCount();
	addr = net::address( addr_from.sin_addr.s_addr,ntohs(addr_from.sin_port) );
	
	data.resize(res);
	memcpy(&data.front(),buffer,res);
	delete	buffer;
	return int(data.size());
	unguard
}

int	socket::canread( int timeout )
{
	// timeout - miliseconds
	guard("socket::canread")
	fd_set	set;
	set.fd_count = 1;
	set.fd_array[0] = s;
	timeval	t_out = {timeout/1000,timeout * 1000};
	int res = select(NULL,&set,NULL,NULL,&t_out);
	if(res == SOCKET_ERROR)
		throw net_exception(WSAGetLastError());
	if(res == 1 ) return true;
	return false;
	unguard
}

int	socket::canwrite(int timeout)
{
	guard("socket::canwrite")
	fd_set	set;
	set.fd_count = 1;
	set.fd_array[0] = s;
	timeval	t_out = {timeout/1000, timeout * 1000};	
	int res = select(NULL,NULL,&set,NULL,&t_out);
	if(res == SOCKET_ERROR)
		throw net_exception(WSAGetLastError());
	if(res == 1) return true;
	return false;
	unguard
}

int	socket::getopt(int level,int optname)
{
	guard("socket::getopt")
	int	value;
	int	length = sizeof(int);
	
	if(getsockopt (s,level,optname,(char*)&value,&length)==SOCKET_ERROR)
		throw net_exception(WSAGetLastError());

	return value;
	unguard
}

int	socket::setopt(int level,int optname,const char* value, int v_len)
{
	guard("socket::setopt(char*)")
	int result = setsockopt (s,level,optname, value, v_len);
	if( result == SOCKET_ERROR )
		throw net_exception(WSAGetLastError());

	return result;
	unguard

}

int	socket::setopt(int level,int optname,int value)
{
	guard("socket::setopt")
	int result = setsockopt (s,level,optname,(char*)&value,sizeof(int));
	if( result == SOCKET_ERROR )
		throw net_exception(WSAGetLastError());

	return result;
	unguard
}

int socket::shutdown( int how )
{
	guard("socket::shutdown")
	int result = ::shutdown( s, how );
	if( result == SOCKET_ERROR )
		throw net_exception(WSAGetLastError());

	return result;
	unguard
}


int socket::close( )
{
	guard("socket::close")
	if( !s || s == SOCKET_ERROR) return 0;
	return closesocket(s);

	unguard
}

int socket::graceful_shutdown ( )
{
	const	int	RECEIVE_TIMEOUT	= 1000;	// milliseconds
	const	int	SEND_TIMEOUT	= 1;	// seconds

	guard("socket::graceful_shutdown")
	shutdown( HL_SD_SEND );

	DWORD StartTimeout = GetTickCount();
	while( true ){
		if( canread( 10 ) && ( receive( std::vector<char>() ) == 0 ) )
			break;
		if( GetTickCount() - StartTimeout >= RECEIVE_TIMEOUT )
			throw "Client has not closed socket within 1 sec";
	}

	// 2 sec for data to be sent before closesocket
	linger	lin = { 1, SEND_TIMEOUT };
	setopt( SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin) );

	close();
	return 1;
	unguard
}

socket::~socket()
{
	guard("socket::~socket")
	close();
	StoptWinSock();
	unguard
}

void socket::set_timeout(dword timeout)
{
	socket::timeout = timeout;
}

int socket::is_timeout()
{
printf("socket::timeout\n");
	return timeout && ( (GetTickCount()-last_transfer) >=timeout);
}
