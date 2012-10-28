#pragma once
#include <stdio.h>

#pragma pack(push,1)

/* 4 bytes IP address */
union ip_address{
	unsigned char	b[4];
	unsigned long	int_val;
	operator	unsigned long(){ return int_val; }

	bool operator == (const ip_address& a)const { return int_val == a.int_val;	}
	bool operator<(const ip_address& a)const { return int_val < a.int_val;	}

	ip_address():int_val(0){}
	const char* c_str()const{
		static char	buffer[100];
		sprintf_s(buffer, sizeof(buffer), "%d.%d.%d.%d",b[0],b[1],b[2],b[3]);
		return buffer;
	}
	ip_address( const char* a ){
		if( 4 != sscanf_s(a,"%d.%d.%d.%d",&b[0],&b[1],&b[2],&b[3]) )throw "ip_address( const char* a )";
	}
	ip_address( unsigned long	ival ):int_val(ival){} //not sure about this ctor. may cause ambiguity or misuse
	ip_address(unsigned char b1,unsigned char b2,unsigned char b3,unsigned char b4){
		b[0]=b1;b[1]=b2;b[2]=b3;b[3]=b4;
	}
};

/* IPv4 header */
struct ip_header{
	unsigned char		ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	unsigned char		tos;			// Type of service 
	unsigned short		tlen;			// Total length 
	unsigned short		identification; // Identification
	unsigned short		flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	unsigned char		ttl;			// Time to live
	unsigned char		proto;			// Protocol
	unsigned short		crc;			// Header checksum
	ip_address	saddr;			// Source address
	ip_address	daddr;			// Destination address
//	unsigned int	op_pad;				// Option + Padding
};

unsigned short ip_csum (unsigned short *buf, long nbytes){
	long	nwords = nbytes /2;
	unsigned long	sum=0;

	for( sum=0; nwords > 0; nwords-- )
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16); 
	return (unsigned short)~sum;
}
#pragma pack(pop)