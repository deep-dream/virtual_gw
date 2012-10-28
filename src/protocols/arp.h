#pragma once
#include "ethernet.h"
#include "ip.h"

#pragma pack(push,1)
// 	ethernet_header - arp_request

// Arp Request (http://www.geocities.com/SiliconValley/Vista/8672/network/arp.html)

struct arp_request{
	unsigned short Hardware_MAC_Address_Type;
	unsigned short Protocol_Address_Type;
	unsigned char	Hardware_MAC_Address_Size;
	unsigned char	Protocol_Address_Size;
	unsigned short	Op;	// 1 (ARP request) 2 (ARP reply) 
	mac_address	sender_mac; //6 bytes (depends on the above size field)
	ip_address	sender_ip; //6 bytes (depends on the above size field)
	mac_address	target_mac; //6 bytes (depends on the above size field)
	ip_address	target_ip; //6 bytes (depends on the above size field)
};

#pragma pack(pop)