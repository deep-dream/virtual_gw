//	Use like this:
//
//	virtual_gw::vgw_header<virtual_gw::discovery>	discovery;
//	vgw.send( discovery );
//
//	if ( vgw.packet_handler(pkt_data, header->len) ) return;

#pragma once
#include <iostream>
#include "protocols/ethernet.h"
#include "protocols/ip.h"
#include "protocols/udp.h"
#include "router.h"
#include "dhcp_client.h"

#include "hl/regexp.h"
#define COMMAND( reg_pattern, code )\
	{regexp match(reg_pattern);		\
	if( match.match(cmd) )			\
	{								\
		std::cout << "COMMAND MATCHED: " << reg_pattern << "\n";\
		code;						\
	}								\
	}								\

#define	VGW_PROTOCOL_ID 0x2A32D435 //707974196
#define	VGW_PORT		2007
#define SERVER_NAME_LEN	32
#define MAX_CMD_LEN		256

namespace VGW{
	enum{
		NONE = 0,
		DISCOVERY,
		REPLY,
		SERVER_COMMAND
	};
}

template<class Transport>
class virtual_gw{
	Transport&					transport;
	router<Transport>&			_router;
	dhcp_client<Transport>&		dhcp_cln;
	std::string					server_name;
public:
	virtual_gw(std::string sname, router<Transport>& r, dhcp_client<Transport>& dcln, Transport& tr)
		:transport(tr),_router(r),dhcp_cln(dcln),server_name(sname){
		std::cout << "Server name: " << server_name << "\n";
	}

	struct none{		enum{ type = VGW::NONE };	};
	struct discovery{	enum{ type = VGW::DISCOVERY }; };

	struct reply{
		enum{ type = VGW::REPLY };
		int			result;
		reply( int res = 0):result(res){}
	};

	struct server_command{
		enum{ type = VGW::SERVER_COMMAND };
		char	command[MAX_CMD_LEN];
		server_command(const std::string& cmd){
			memcpy(command, cmd.c_str(), min(cmd.size()+1,sizeof(command)) );
		}
	};

	template<class T = none>
	struct vgw_header{
		unsigned int	protocol_id;
		char			server_name[SERVER_NAME_LEN];
		unsigned int	request_id;		
		unsigned char	type;
		T				data;
		vgw_header(const std::string& sn="", const T& d = T() )
		:protocol_id(VGW_PROTOCOL_ID), request_id(urid()), type(T::type), data(d){
			memcpy(server_name, sn.c_str(), min(sn.size()+1,sizeof(server_name)) );
		}
		int urid(){ return rand(); }
	};
	
	template<class T = vgw_header<> >
	struct check_packet{
		const vgw_header<T>& packet; 
		check_packet( const vgw_header<>& base_packet):packet(reinterpret_cast<const vgw_header<T>&>(base_packet)){}
		operator bool(){ return packet.type == T::type; }
		const T* operator ->(){ return &packet.data; }
		const vgw_header<T>& header(){ return packet; }
	};

	
	int dispatch(const unsigned char* data, int size){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*data);		
		if( eh.ether_type != htons(0x0800)) return 0;

		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		if( iph.proto != IPPROTO_UDP ) return 0;

		int ip_len = (iph.ver_ihl & 0xf) * 4;
		const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));
		if( ntohs(udph.dport) != VGW_PORT ) return 0;

		const vgw_header<>& vgwh = reinterpret_cast<const vgw_header<>&>(udph.data);
		if( vgwh.protocol_id != VGW_PROTOCOL_ID ) return 0;					

		if( std::string(vgwh.server_name) != server_name ){
			std::cout << "Wrong server name:"<< vgwh.server_name << "\n";
			return 1;
		}
		
		int result = exec(vgwh);
		send( eh.source, dhcp_cln.get_ip(), iph.saddr, ntohs(udph.sport), vgw_header<reply>( server_name, result) );

		return 1;
	}

	int exec(const vgw_header<>& vgwh){
		if( check_packet<discovery> packet = vgwh ){
			std::cout << "VGW Discovery(where is "<< packet.header().server_name << ")\n";
			return 1;
		}

		if( check_packet<server_command> packet = vgwh ){
			std::cout << "VGW server_command:"<< packet->command << "\n";
			const char* cmd = packet->command; // for macros COMMAND

			try{
				COMMAND( "^[[:space:]]*stop[[:space:]]*$", 
					Ctrl_break_handler::ctrl_break_pressed = true; 
				)

				COMMAND( "q:([-_A-Za-z0-9]+) +(.*)",
					std::string q_key = match[1];
					std::string cmd = match[2];
					COMMAND( "^reset",
						_router.qm.clear_queue(q_key);
					)
					COMMAND( "^qsize +([0-9]+)",
						_router.qm.set_qsize( q_key, atoi(match[1].c_str()) );
					)
					COMMAND( "^filter +(.*)", 
						_router.qm.set_filter(q_key,match[1]); 
					)

					COMMAND( "^set:([0-9]+) +([a-z]+) +(.+)",
						_router.qm.set(q_key, match[1], match[2], match[3]);
					)

					COMMAND( "^push_back +([a-z]+) +(.+)",
						_router.qm.push_back_qdisc(q_key, match[1], match[2]);
					)
				)// Q:

				COMMAND( " *print", 
					_router.qm.print_all(); 
				)
				return 1;
			}catch(const std::exception& ){
				std::cout << "Error while executing command\n";
				return 0;
			}
		}
		return 0;
	}

	
	template<class T>
	void send( const mac_address& d_mac,const ip_address& s_ip,const ip_address& d_ip, unsigned short udp_dport, const vgw_header<T>& vgwh_out )
	{
		mac_address broadcast_mac(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
//		ip_address  broadcast_ip(255,255,255,255);
		unsigned short udp_sport = VGW_PORT;
//		unsigned short udp_dport = 100;
		//---------------------------------------------------------------
		unsigned char buffer[1024]={0};
		ethernet_header& eh = reinterpret_cast<ethernet_header&>(*buffer);

		eh.dest			= d_mac;
		eh.source		= d_mac;
		eh.ether_type	= htons(0x0800); //ip

		ip_header& iph = reinterpret_cast<ip_header&>(*eh.data);

		iph.ver_ihl = 0x45;	
		iph.ttl		= 128;
		iph.daddr	= d_ip;
		iph.saddr	= s_ip;
		iph.proto	= IPPROTO_UDP;

		udp_header& udph = reinterpret_cast<udp_header&>(*(eh.data + 20));	

		udph.sport = htons(udp_sport);
		udph.dport = htons(udp_dport);

		vgw_header<T>& vgwh = reinterpret_cast<vgw_header<T>&>(udph.data);
		vgwh = vgwh_out;
		//vgwh.command = commands::cmd_status;
		
		unsigned char* end_of_packet = (unsigned char*)&vgwh + sizeof(vgwh);
		//------- 
		unsigned long size = (unsigned long)( end_of_packet - buffer);
		udph.total_length = htons(end_of_packet - (unsigned char*)&udph);
		udph.checksum = udp_csum ( (unsigned short*)&udph, ntohs(udph.total_length), iph.saddr.int_val,iph.daddr.int_val);

		iph.tlen= htons(size - sizeof(ethernet_header));
		iph.crc = 0;
		iph.crc = ip_csum((unsigned short*)(buffer+sizeof(ethernet_header)), 20 );

		transport.send( (const unsigned char*)buffer, size );
	}
};