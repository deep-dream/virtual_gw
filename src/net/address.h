/*=============================================================================
    "HeadLong Library"
    ______________________

    address.h:	(created 2001/02/02 	0:05)
  
        Description:
			
	
	  
        Copyright (c) Artyom V. Borodkin, 2001.  All Rights Reserved.
                                                   (http://www.deep-dream.com)
=============================================================================*/
#pragma once
#include <string>

namespace net{

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned int* dwordp;

	
class address{	
	dword	_host;
	word	_port;
public:
	address(dword host = 0,word port = 0):_host(host), _port(port){}
	address(std::string host,word port);	

	std::string IPAsString()const;
	word		port()const;
	dword		host()const;
};
}

