#pragma once
#include <winsock2.h>

#pragma pack(push,1)

// http://en.wikipedia.org/wiki/User_Datagram_Protocol
// http://www.ietf.org/rfc/rfc768.txt

struct udp_header{
	unsigned short	sport;
	unsigned short	dport;
	unsigned short	total_length;
	unsigned short	checksum;
	unsigned char	data[];
};

unsigned short udp_csum (unsigned short  *buf, int nbytes, unsigned long saddr,unsigned long daddr){
	int nwords = nbytes /2;
	struct pseudo_hdr {			/* See RFC 793 Pseudo Header */ 
		unsigned long saddr, daddr;	/* source and dest address */ 
		unsigned char mbz, ptcl;		/* zero and protocol */ 
		unsigned short length;			/* length */ 
	} ph;
	ph.saddr = saddr;
	ph.daddr = daddr;
	ph.mbz = 0;
	ph.ptcl = IPPROTO_UDP;
	ph.length = htons(nbytes);

	unsigned long sum  = 0;
	unsigned short*	pointer = (unsigned short*)&ph;

	for( int i=0; i < sizeof(ph)/2; ++i)
		sum += *pointer++;

	pointer = buf;

	for(int i=0; i < nwords; ++i)
		sum += *pointer++;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16); 
	return (unsigned short)~sum;
}

#pragma pack(pop)
