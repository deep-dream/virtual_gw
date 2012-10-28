#pragma once
#include <assert.h>
#include "protocols/ethernet.h"
#include "protocols/ip.h"
#include "protocols/arp.h"
#include "protocols/udp.h"
#include "protocols/dhcp.h"

#define DHCPDISCOVER	1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

template<class Transport>
class dhcp_client{
	Transport&			transport;
public:
	ip_address	_mask;
	ip_address	_ip;
	ip_address	_server;
	unsigned char		_is_configured;
	mac_address _mac;
	unsigned long		_xid;

public:
	const  ip_address&	get_ip(){ return _ip;}

	dhcp_client(const mac_address& mac, Transport& tr ):
		_is_configured(0),_mac(mac), transport(tr){
			srand( time(0) );
			_xid = rand();
		}
	~dhcp_client(){ send_release(); }
	bool dispatch(const unsigned char *packet, const int len){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*packet);
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);

		if( eh.ether_type == htons(0x0800) && iph.proto == IPPROTO_UDP ){
			int ip_len = (iph.ver_ihl & 0xf) * 4;
			const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));	
			const dhcp_header& dhcph = reinterpret_cast<const dhcp_header&>(*((char*)&udph + sizeof(udp_header)));
			ip_address server,mask;
			unsigned char dhcp_message_type = 0;
			if (dhcph.magic_cookie == ntohl(0x63825363) ) {
				if( dhcph.op == 2 ){
					printf("DHCP Reply ");

					if ( dhcph.xid != _xid ){
						printf(" - not our offer\n");
						return true;
					}
					// parse options
					for (int i=0;i<300;){
						switch (dhcph.options[i]){
						case 0xFF: i=9999; break;
						case 53: // message type
							dhcp_message_type = dhcph.options[i+2];
							break;
						case 54: // server identifier
							server.int_val = *(unsigned long*)(dhcph.options+i+2);
							break;
						case 01: //subnet mask
							mask.int_val = *(unsigned long*)(dhcph.options+i+2);
							break;
						}
						i += dhcph.options[i+1]+2;
					}
					// take actions
					switch (dhcp_message_type){
					case DHCPOFFER:
						// store offered parameters
						_ip		= dhcph.yiaddr;
						_server	= server;
						_mask	= mask;
						printf(" - Offer (offered ip %s)\n", ip_address(dhcph.yiaddr).c_str() );
						send_request();
						break;
					case DHCPACK:
						printf(" - Acknowledgment\n");
						_is_configured = 1;
//						send_release();
						break;
					case DHCPNAK:
						printf(" - Negative Acknowledgment (Warning)\n");
						break;
					}

				}			
			}		
		}
		return false;
	}

	void send_discover(){
		send_packet(DHCPDISCOVER);
		_is_configured = 0;
	}

	void send_request(){
		send_packet(DHCPREQUEST);
		_is_configured = 0;
	}

	void send_release(){
		if(_is_configured) send_packet(DHCPRELEASE);
		_is_configured = 0;
	}
/*
	void packet_handler(unsigned char *param, const struct pcap_pkthdr *header, const unsigned char *pkt_data){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*pkt_data);
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);

		if( eh.ether_type == htons(0x0800) && iph.proto == IPPROTO_UDP ){
			int ip_len = (iph.ver_ihl & 0xf) * 4;
			const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));	
			const dhcp_header& dhcph = reinterpret_cast<const dhcp_header&>(*((char*)&udph + sizeof(udp_header)));
			ip_address server,mask;
			unsigned char dhcp_message_type = 0;
			if (dhcph.magic_cookie == ntohl(0x63825363) ) {
				if( dhcph.op == 2 ){
					printf("DHCP Reply ");

					if ( dhcph.xid != _xid ){
						printf(" - not our offer\n");
						return;
					}
					// parse options
					for (int i=0;i<300;){
						switch (dhcph.options[i]){
						case 0xFF: i=9999; break;
						case 53: // message type
							dhcp_message_type = dhcph.options[i+2];
							break;
						case 54: // server identifier
							server.int_val = *(unsigned long*)(dhcph.options+i+2);
							break;
						case 01: //subnet mask
							mask.int_val = *(unsigned long*)(dhcph.options+i+2);
							break;
						}
						i += dhcph.options[i+1]+2;
					}
					// take actions
					switch (dhcp_message_type){
					case DHCPOFFER:
						// store offered parameters
						_ip		= dhcph.yiaddr;
						_server	= server;
						_mask	= mask;
						printf(" - Offer (offered ip %s)\n", ip2str(dhcph.yiaddr) );
						send_request();
						break;
					case DHCPACK:
						printf(" - Acknowledgment\n");
						_is_configured = 1;
//						send_release();
						break;
					case DHCPNAK:
						printf(" - Negative Acknowledgment (Warning)\n");
						break;
					}

				}			
			}		
		}
	}
*/	
protected:
	void send_packet(unsigned char dhcp_message_type)
	{
		unsigned char buffer[1024]={0};

		ethernet_header& eh = reinterpret_cast<ethernet_header&>(*buffer);

//		mac_address fake_mac={0x00,0x04,0x23,0x04,0x05,0x06};
//		mac_address fake_mac(0x00,0x05,0x23,0x04,0x05,0x06);
		mac_address broadcast_mac(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
		ip_address  broadcast_ip(255,255,255,255);

		eh.dest			= broadcast_mac;
		eh.source		= _mac;
		eh.ether_type	= htons(0x0800); //ip

		ip_header& iph = reinterpret_cast<ip_header&>(*eh.data);	

		iph.ver_ihl = 0x45;	
		iph.ttl		= 128;
		iph.daddr	= broadcast_ip;
		iph.proto	= IPPROTO_UDP;

		udp_header& udph = reinterpret_cast<udp_header&>(*(eh.data + 20));	

		udph.sport = htons(68);
		udph.dport = htons(67);

		dhcp_header& dhcph = reinterpret_cast<dhcp_header&>(*((char*)&udph + sizeof(udp_header)));

		dhcph.op	= 1;
		dhcph.htype	= 1;
		dhcph.hlen	= 6;
		dhcph.hops	= 0;
		dhcph.xid	= _xid;
		memcpy(&dhcph.chaddr,&_mac,sizeof(_mac));
		dhcph.magic_cookie = htonl(0x63825363);
		unsigned char *opt = dhcph.options;
		switch (dhcp_message_type){
			case DHCPDISCOVER:
				// DHCP Message Type
				*opt++ = 53;
				*opt++ = 1;
				*opt++ = dhcp_message_type;
				break;
			case DHCPREQUEST:
			case DHCPRELEASE:
				// DHCP Message Type
				*opt++ = 53;
				*opt++ = 1;
				*opt++ = dhcp_message_type;
				// Requested IP Address
				*opt++ = 50;
				*opt++ = 4;
				memcpy(opt, &_ip.int_val, 4);
				opt +=4;
				// Server Identifier
				*opt++ = 54;
				*opt++ = 4;
				memcpy(opt, &_server.int_val, 4);
				opt +=4;
				break;
			default:
				assert(!"Unknown message type");
		}		
		// End Option
		*opt++ = 255;
		*opt   = 0;
		unsigned long size = (unsigned long)(opt - buffer);
		assert(size <= sizeof(buffer));

		//------- 
		udph.total_length = htons((char*)opt - (char*)&udph);
		udph.checksum = udp_csum ( (unsigned short*)&udph, ntohs(udph.total_length), iph.saddr.int_val,iph.daddr.int_val);

		iph.tlen= htons(size - sizeof(ethernet_header));
		iph.crc = 0;
		iph.crc = ip_csum((unsigned short*)(buffer+sizeof(ethernet_header)), 20 );

		switch (dhcp_message_type){
		case DHCPDISCOVER:	printf("Sending DHCPDISCOVER\n");break;
		case DHCPREQUEST:	printf("Sending DHCPREQUEST\n");break;
		case DHCPRELEASE:	printf("Sending DHCPRELEASE\n");break;
		}
		transport.send((const unsigned char*)buffer, size );
	}
};