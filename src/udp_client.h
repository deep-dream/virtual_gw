#pragma once
#include <map>
#include <assert.h>
#include "protocols/ethernet.h"
#include "protocols/ip.h"
#include "protocols/arp.h"
#include "protocols/udp.h"
#include "protocols/dhcp.h"

enum _UDP_CLIENT_COMMANDS{
DISCOVERY=0,
SET_RULE,
GET_RULE,
LIST_RULES
};

#define VIRTUAL_GW_PORT 12345
/*
class ip_client{
	ip_address local_ip;
public:
	ip_client(ip_address &ip):local_ip(ip){}
	~ip_client(){}
};

class ethernet_client{

public:
	ethernet_client(){}
	~ethernet_client(){}
};
*/
class udp_client{
	ip_address	_ip;
public:

	udp_client(){}
	~udp_client(){}

	void set_ip(ip_address ip){ _ip = ip; }

	int packet_handler(unsigned char *param, const struct pcap_pkthdr *header, const unsigned char *pkt_data)
	{
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*pkt_data);

		if( eh.ether_type != htons(0x0800) ) // !IP
			return 0; // not our proto

		const ip_header* iph = (const ip_header*)eh.data;

		if( iph->daddr == _ip || iph->daddr == ip_address(0xFFFFFFFF) ){
			printf("%s ->", iph->saddr.c_str());
			printf("%s\n", iph->daddr.c_str());
			int hl = sizeof(ip_header)+sizeof(ethernet_header);
			return process(pkt_data+hl, header->len-hl); //UDP header+data
		}
		return 0;
	}

protected:
	int process(const unsigned char *pkt_data, const unsigned int len)
	{
		const udp_header& udph = reinterpret_cast<const udp_header&>(*pkt_data);
		if (udph.dport != VIRTUAL_GW_PORT) return;
		switch(udph.data[0])
		{
		case DISCOVERY:
			break;
		case SET_RULE:
			break;
		case GET_RULE:
			break;
		case LIST_RULES:
			break;
		}
	}
	void send_discovery(ip_address target_ip, )
	{
	}
/*
	void send(ip_address target_ip, const unsigned char *pkt_data, unsigned int len)
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

		printf("Sending ARP Reply to %s (our MAC is %x-%x-%x-%x-%x-%x)\n",ip2str(arph.target_ip),
			_mac.byte1,_mac.byte2,_mac.byte3,_mac.byte4,_mac.byte5,_mac.byte6);
		if (pcap_sendpacket(adhandle, (const unsigned char*)buffer, sizeof(arp_request)+sizeof(ethernet_header) ) != 0){
			fprintf(stderr,"\nError sending the packet: \n", pcap_geterr(adhandle));
		}		
	}
*/
};