/*=============================================================================
    "HeadLong Library"
    ______________________

    net.h:	(created 2001/02/01 	13:39)
  
        Description:
			
	
	  
        Copyright (c) Artyom V. Borodkin, 2001.  All Rights Reserved.
                                                   (http://www.deep-dream.com)
=============================================================================*/
#pragma once
#include <string>
#include <windows.h>

//#include "hl/exception.h"
#ifndef guard
	#define guard(x)
	#define unguard
#endif

#ifdef WINCE
	#pragma comment(lib, "Ws2.lib")
#else
	#pragma comment(lib,"Ws2_32.lib")
#endif



#include "address.h"
#include "socket.h"
#define SO_MAX_MSG_SIZE   0x2003      /* maximum message size */

#ifndef AF_INET
	#define AF_INET 2
	#define SOCK_STREAM 1
	#define IPPROTO_TCP 6
	#define INADDR_ANY              (unsigned long)0x00000000
	#define SOCKET_ERROR            (-1)
#endif

//#pragma comment(lib,"Ws2_32.lib")

namespace	net{
	class net_exception : public std::exception{
		std::string		message;
		const char *what() const { return message.c_str(); }
	public:
		net_exception(const std::string& msg) : message( msg.c_str() ) {}
		net_exception(int last_err) : message("") {

		LPVOID lpMsgBuf = NULL;
		switch(last_err){
			case WSANOTINITIALISED: lpMsgBuf = "A successful WSAStartup call must occur before using this function.";break;
			case WSAENETDOWN: lpMsgBuf = "The network subsystem has failed.";break;
			case WSAEFAULT: lpMsgBuf = "The buf or from parameters are not part of the user address space, or the fromlen parameter is too small to accommodate the peer address.";break;
			case WSAEINTR: lpMsgBuf = "The (blocking) call was canceled through WSACancelBlockingCall.";break;
			case WSAEINPROGRESS: lpMsgBuf = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";break;
			case WSAEINVAL: lpMsgBuf = "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled, or (for byte stream-style sockets only) len was zero or negative.";break;
			case WSAEISCONN: lpMsgBuf = "The socket is connected. This function is not permitted with a connected socket, whether the socket is connection-oriented or connectionless.";break;
			case WSAENETRESET: lpMsgBuf = "The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.";break;
			case WSAENOTSOCK: lpMsgBuf = "The descriptor is not a socket.";break;
			case WSAEOPNOTSUPP: lpMsgBuf = "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.";break;
			case WSAESHUTDOWN: lpMsgBuf = "The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.";break;
			case WSAEWOULDBLOCK: lpMsgBuf = "The socket is marked as nonblocking and the recvfrom operation would block.";break;
			case WSAEMSGSIZE: lpMsgBuf = "The message was too large to fit into the specified buffer and was truncated.";break;
			case WSAETIMEDOUT: lpMsgBuf = "The connection has been dropped, because of a network failure or because the system on the other end went down without notice.";break;
			case WSAECONNRESET: lpMsgBuf = "The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.";break;
			default: lpMsgBuf = NULL; break;
		}	
			if(last_err && !lpMsgBuf){
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					last_err,
					0, // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
				if(lpMsgBuf){
					// Process any inserts in lpMsgBuf.
					message = (LPCSTR)lpMsgBuf;

					// Free the buffer.
					LocalFree( lpMsgBuf );
				}
			}else{
				message = (LPCSTR)lpMsgBuf;
			}
			printf("%s\n",message.c_str());
		}
	};
}