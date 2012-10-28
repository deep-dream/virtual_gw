#pragma once

#pragma pack(push,1)

struct	mac_address{
	unsigned char byte1, byte2, byte3, byte4, byte5, byte6;
	
	mac_address():
		byte1(0),byte2(0),byte3(0),byte4(0),byte5(0),byte6(0){}
	mac_address(unsigned char b1,unsigned char b2,unsigned char b3,unsigned char b4,unsigned char b5,unsigned char b6):
		byte1(b1),byte2(b2),byte3(b3),byte4(b4),byte5(b5),byte6(b6){}

	bool operator == (const mac_address& a)const { 
		return (
			byte1 == a.byte1 && 
			byte2 == a.byte2 && 
			byte3 == a.byte3 && 
			byte4 == a.byte4 && 
			byte5 == a.byte5 && 
			byte6 == a.byte6
			);
	}
};

/* 10Mb/s ethernet header */
struct ethernet_header{
	mac_address	dest;
	mac_address	source;
	unsigned short ether_type;
	char		data[];
};

/*	EtherType
	0x0600 XNS Internet Datagram Protocol (IDP) 
	0x0800 IP Internet Protocol (IPv4) 
	0x0806 Address Resolution Protocol (ARP) 
	0x8035 Reverse Address Resolution Protocol (RARP) 
	0x809b AppleTalk (Ethertalk) 
	0x80f3 Appletalk Address Resolution Protocol (AARP) 
	0x8100 (identifies IEEE 802.1Q-tagged frame) 
	0x8137 Novell IPX (alt) 
	0x8138 Novell 
	0x86DD Internet Protocol, Version 6 (IPv6) 
	0x8847 MPLS unicast 
	0x8848 MPLS multicast 
	0x8863 PPPoE Discovery Stage 
	0x8864 PPPoE Session Stage
*/

#pragma pack(pop)