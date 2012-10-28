#pragma once
#include "virtual_gw.h"	
#include "hl/params.h"
#include "net/net.h"

class vgw_client{
	net::socket s;
	std::string	server_name;
	params all_params;
public:
	vgw_client():s(AF_INET,SOCK_DGRAM,0),server_name("unknown"),all_params( GetCommandLine() )
	{
		all_params.read("-name=([-_A-Za-z0-9]+)", server_name, false);
		s.setopt(SOL_SOCKET,SO_BROADCAST,TRUE);
		s.bind( net::address(INADDR_ANY, VGW_PORT+1 ) );
	}

	template<class T>
	int request(const T& request){
		int n = 4;
		all_params.read("-t=([0-9]+)", n, false);
		for(int i=0;i<n;++i){
			std::cout << "Send request to " << server_name << "\n";
			s.sendto(net::address( INADDR_BROADCAST, VGW_PORT ), std::vector<char>((char*)&request,(char*)&request+sizeof(request)) );
			if( s.canread( 1000 ) ){
				  net::address  client_addr;
				  std::vector<char>   data;
				  s.receivefrom( client_addr, data );
				  std::cout << "\nMessage from: "<< client_addr.IPAsString() << std::endl;
				  return dispatch(&data[0]);
			}
		}
		return 0;
	}

	int dispatch(const char *data){//const std::vector<char>   &data){
		const virtual_gw<vgw_client>::vgw_header<>& vgwh = reinterpret_cast<const virtual_gw<vgw_client>::vgw_header<>&>(*data);
		if( vgwh.protocol_id != VGW_PROTOCOL_ID ) return 0;

		if( virtual_gw<vgw_client>::check_packet<virtual_gw<vgw_client>::reply> packet = vgwh ){
			std::cout << "VGW reply server:" << packet.header().server_name << ", result: " << packet->result << "\n";
			return packet->result;
		}
		return 0;
	}

	int discovery(){
		virtual_gw<vgw_client>::vgw_header<virtual_gw<vgw_client>::discovery>	discovery(server_name);
		return request(discovery);
	}

	int server_command(){
		std::string	command;
		all_params.read("-scmd(.*)", command, false);
		std::cout << "Sending server command \""<< command << "\"\n";
		virtual_gw<vgw_client>::vgw_header<virtual_gw<vgw_client>::server_command>
			scmd(server_name, command);
		return request(scmd);
	}
};
