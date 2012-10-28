#pragma once
#include <map>
#include <assert.h>
#include "protocols/ethernet.h"
#include "protocols/ip.h"
#include "protocols/arp.h"
#include "protocols/udp.h"
#include "protocols/dhcp.h"

template<class Transport>
class arp_client{
	Transport&			transport;
	ip_address	_mask;
	ip_address	_ip;
	mac_address _mac;
	std::map<ip_address,mac_address>	_arp_table;
public:

	arp_client(const mac_address& mac, Transport& tr ):_mac(mac), transport(tr){}
	~arp_client(){}

	bool dispatch(const unsigned char *packet, const int len){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*packet);

		if( eh.ether_type == htons(0x0806) ){
			const arp_request* arph = (const arp_request*)eh.data;
			// ARP Request
			if( arph->Op == htons(1) && arph->target_ip == _ip ){
				printf("ARP request from %s\n", arph->sender_ip.c_str());
				send_reply(arph->sender_mac, arph->sender_ip);
				return true;
			}

			// ARP Reply
			if( arph->Op == htons(2) ){
				printf("ARP reply(%s has %x-%x-%x-%x-%x-%x MAC)\n", arph->sender_ip.c_str(),
					arph->sender_mac.byte1,arph->sender_mac.byte2,arph->sender_mac.byte3,arph->sender_mac.byte4,
					arph->sender_mac.byte5,arph->sender_mac.byte6
					);
				_arp_table[arph->sender_ip] = arph->sender_mac;
				return true;
			}
		}	
		return false;
	}
	void set_ip(ip_address ip){ _ip = ip; }

	bool get_mac(const ip_address& ip, mac_address& mac)
	{
		std::map<ip_address,mac_address>::iterator tmac = _arp_table.find(ip);
		if(tmac == _arp_table.end())
		{
			printf("Error no MAC for target %s\n",ip.c_str());
			send_request(ip);
			return false;
		}else{
			mac = tmac->second;
		}		
		return true;
	}
/*
	void packet_handler(unsigned char *param, const struct pcap_pkthdr *header, const unsigned char *pkt_data)
	{
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*pkt_data);

		if( eh.ether_type != htons(0x0806) )
			return; // not our proto
		{
			const arp_request* arph = (const arp_request*)eh.data;
			// ARP Request
			if( arph->Op == htons(1) && arph->target_ip == _ip ){
				printf("ARP request from %s\n", ip2str(arph->sender_ip));
				send_reply(arph->sender_mac, arph->sender_ip);
			}

			// ARP Reply
			if( arph->Op == htons(2) ){
				printf("ARP reply(%s has %x-%x-%x-%x-%x-%x MAC)\n", ip2str(arph->sender_ip),
					arph->sender_mac.byte1,arph->sender_mac.byte2,arph->sender_mac.byte3,arph->sender_mac.byte4,
					arph->sender_mac.byte5,arph->sender_mac.byte6
					);
				_arp_table[arph->sender_ip] = arph->sender_mac;
			}
		}	
	}
*/
protected:
	void send_reply(mac_address target_mac, ip_address target_ip)
	{
		static char buffer[1024];

		ethernet_header& eh = reinterpret_cast<ethernet_header&>(buffer);
		arp_request& arph = reinterpret_cast<arp_request&>(eh.data);

		eh.dest		= target_mac;
		eh.source	= _mac;

		eh.ether_type = htons(0x0806);

		arph.Hardware_MAC_Address_Type = htons(1);
		arph.Protocol_Address_Type = htons(0x0800);
		arph.Hardware_MAC_Address_Size = 6;
		arph.Protocol_Address_Size = 4;
		arph.Op = htons(2);
		arph.sender_mac = _mac;
		arph.sender_ip = _ip;

		arph.target_mac	= target_mac;
		arph.target_ip	= target_ip;

		printf("Sending ARP Reply to %s (our MAC is %x-%x-%x-%x-%x-%x)\n",arph.target_ip.c_str(),
			_mac.byte1,_mac.byte2,_mac.byte3,_mac.byte4,_mac.byte5,_mac.byte6);
		transport.send((const unsigned char*)buffer, sizeof(arp_request)+sizeof(ethernet_header) );
	}
	void send_request(ip_address target_ip)
	{
		static char buffer[1024];

		ethernet_header& eh = reinterpret_cast<ethernet_header&>(buffer);
		arp_request& arph = reinterpret_cast<arp_request&>(eh.data);

		mac_address broadcast_mac(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

		eh.dest		= broadcast_mac;
		eh.source	= _mac;

		eh.ether_type = htons(0x0806);

		arph.Hardware_MAC_Address_Type = htons(1);
		arph.Protocol_Address_Type = htons(0x0800);
		arph.Hardware_MAC_Address_Size = 6;
		arph.Protocol_Address_Size = 4;
		arph.Op = htons(1);
		arph.sender_mac = _mac;
		arph.sender_ip = _ip;

		arph.target_mac = broadcast_mac;
		arph.target_ip = target_ip;

		printf("Sending ARP Request(Who has %s)\n",arph.target_ip.c_str());
		transport.send((const unsigned char*)buffer, sizeof(arp_request)+sizeof(ethernet_header) );
	}
};