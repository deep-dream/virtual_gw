#pragma once
#include <winsock2.h>

#pragma pack(push,1)

#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20

enum{
	TCP_ESTABLISHED = 1,
	TCP_SYN_SENT,
	TCP_SYN_RECV,
	TCP_FIN_WAIT1,
	TCP_FIN_WAIT2,
	TCP_TIME_WAIT,
	TCP_CLOSE,
	TCP_CLOSE_WAIT,
	TCP_LAST_ACK,
	TCP_LISTEN,
	TCP_CLOSING   // now a valid state 
};

#define	TCPOPT_EOL		0
#define	TCPOPT_NOP		1
#define	TCPOPT_MAXSEG		2
#define TCPOLEN_MAXSEG		4
#define TCPOPT_WINDOW		3
#define TCPOLEN_WINDOW		3
#define TCPOPT_SACK_PERMITTED	4		// Experimental 
#define TCPOLEN_SACK_PERMITTED	2
#define TCPOPT_SACK		5		// Experimental 
#define TCPOPT_TIMESTAMP	8
#define TCPOLEN_TIMESTAMP	10
#define TCPOLEN_TSTAMP_APPA	(TCPOLEN_TIMESTAMP+2) // appendix A 

#define TCPOPT_TSTAMP_HDR (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)


// * Default maximum segment size for TCP.
// * With an IP MSS of 576, this is 536,
// * but 512 is probably more convenient.
// * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).

#define	TCP_MSS	512
#define	TCP_MAXWIN	65535	// largest value for (unscaled) window 
#define TCP_MAX_WINSHIFT	14	// maximum window shift 


// * User-settable options (used with setsockopt).

//#define	TCP_NODELAY	0x01	// don't delay send to coalesce packets 
#define	TCP_MAXSEG	0x02	// set maximum segment size 
#define TCP_CORK	0x03	// control sending of partial frames 

#define SOL_TCP		6	// TCP level 

typedef unsigned short	byte2;
typedef unsigned int	byte4;

struct tcphdr {
	byte2 th_sport;
	byte2 th_dport;
	byte4 seq;
	byte4 ack_seq;

	union{ 
		byte2 th_flags;
		struct{
			byte2 res1:4;
			byte2 doff:4;
			byte2 fin:1;
			byte2 syn:1;
			byte2 rst:1;
			byte2 psh:1;
			byte2 ack:1;
			byte2 urg:1;
			byte2 res2:2;
		};
	};
	byte2 window;
	byte2 check;
	byte2 urg_ptr;
};

unsigned short tcp_csum (unsigned short *buf, int nbytes, unsigned char* saddr,unsigned char* daddr){
	int i,nwords = nbytes /2;
	struct pseudo_hdr {			/* See RFC 793 Pseudo Header */ 
		unsigned long saddr, daddr;	/* source and dest address */ 
		unsigned char mbz, ptcl;		/* zero and protocol */ 
		unsigned short tcpl;			/* tcp length */ 
	} ph;
	ph.saddr = *(unsigned long*)saddr;
	ph.daddr = *(unsigned long*)daddr;
	ph.mbz = 0;
	ph.ptcl = IPPROTO_TCP;
	ph.tcpl = htons(nbytes);

	unsigned long sum  = 0;
	unsigned short*	pointer = (unsigned short*)&ph;

	for( int i=0; i < sizeof(ph)/2; ++i)
		sum += *pointer++;
	
	pointer = buf;

	for( i=0; i < nwords; ++i)
		sum += *pointer++;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16); 
	return (unsigned short)~sum;
}

#pragma pack(pop)