#pragma once

#pragma pack(push,1)

// http://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
// http://www.ietf.org/rfc/rfc2131.txt
// http://www.ietf.org/rfc/rfc2132.txt


// DHCP uses the same two IANA assigned ports for BOOTP: 67/udp for the server side, and 68/udp for the client side.

struct dhcp_header{
	unsigned char	op;			// Message op code / message type.	1 = BOOTREQUEST, 2 = BOOTREPLY
	unsigned char	htype;		// Hardware address type, see ARP section in "Assigned 	Numbers" RFC; e.g., '1' = 10mb ethernet.
	unsigned char	hlen;		// Hardware address length (e.g.  '6' for 10mb	ethernet).	
	unsigned char	hops;		// Client sets to zero, optionally used by relay agents when booting via a relay agent.
	unsigned long	xid;		// Transaction ID, a random number chosen by the client, used by the client and server to associate messages and responses between a client and a server.
	unsigned short	secs;		// Filled in by client, seconds elapsed since client 	began address acquisition or renewal process.
	unsigned short	flags;		// Flags (see figure 2).
	unsigned long	ciaddr;		// Client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to ARP requests.
	unsigned long	yiaddr;		// 'your' (client) IP address.
	unsigned long	siaddr;		// IP address of next server to use in bootstrap; returned in DHCPOFFER, DHCPACK by server.
	unsigned long	giaddr;		// Relay agent IP address, used in booting via a relay agent.
	unsigned char	chaddr[16];	// Client hardware address.
	unsigned char	sname[64];	// Optional server host name, null terminated string.
	unsigned char	file[128];	// Boot file name, null terminated string; "generic" name or null in DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER.
	unsigned long	magic_cookie;// 0x63825363
	unsigned char	options[];	// Optional parameters field.  See the options documents for a list of defined options.
};
#pragma pack(pop)

/*
				v               v               v
				|               |               |
				|     Begins initialization     |
				|               |               |
				| _____________/|\____________  |
				|/DHCPDISCOVER  | DHCPDISCOVER \|
				|               |               |
				Determines      |        Determines
				configuration   |       configuration
				|               |               |
				|\              |  ____________/|
				| \________     | /DHCPOFFER    |
				| DHCPOFFER\    |/              |
				|            \  |               |
				|       Collects replies        |
				|              \|               |
				|     Selects configuration     |
				|               |               |
				| _____________/|\____________  |
				|/ DHCPREQUEST  |  DHCPREQUEST\ |
				|               |               |
				|               |     Commits configuration
				|               |               |
				|               | _____________/|
				|               |/ DHCPACK      |
				|               |               |
				|    Initialization complete    |
				|               |               |
				.               .               .
				.               .               .
				|               |               |
				|      Graceful shutdown        |
				|               |               |
				|               |\ ____________ |
				|               | DHCPRELEASE  \|
				|               |               |
				|               |        Discards lease
				|               |               |
				v               v               v
	Timeline diagram of messages exchanged between DHCP
	client and servers when allocating a new network address
*/

/*3.1. Pad Option

The pad option can be used to cause subsequent fields to align on
word boundaries.

The code for the pad option is 0, and its length is 1 octet.

Code
+-----+
|  0  |
+-----+
*/

/*
3.2. End Option

The end option marks the end of valid information in the vendor
field.  Subsequent octets should be filled with pad options.

The code for the end option is 255, and its length is 1 octet.

Code
+-----+
| 255 |
+-----+
*/

/*
9.6. DHCP Message Type

This option is used to convey the type of the DHCP message.  The code
for this option is 53, and its length is 1.  Legal values for this
option are:

Value   Message Type
-----   ------------
1     DHCPDISCOVER
2     DHCPOFFER
3     DHCPREQUEST
4     DHCPDECLINE
5     DHCPACK
6     DHCPNAK
7     DHCPRELEASE
8     DHCPINFORM

Code   Len  Type
+-----+-----+-----+
|  53 |  1  | 1-9 |
+-----+-----+-----+
*/

/*
9.1. Requested IP Address

This option is used in a client request (DHCPDISCOVER) to allow the
client to request that a particular IP address be assigned.

The code for this option is 50, and its length is 4.

Code   Len          Address
+-----+-----+-----+-----+-----+-----+
|  50 |  4  |  a1 |  a2 |  a3 |  a4 |
+-----+-----+-----+-----+-----+-----+

3.3. Subnet Mask

The subnet mask option specifies the client's subnet mask as per RFC
950 [5].

If both the subnet mask and the router option are specified in a DHCP
reply, the subnet mask option MUST be first.

The code for the subnet mask option is 1, and its length is 4 octets.

Code   Len        Subnet Mask
+-----+-----+-----+-----+-----+-----+
|  1  |  4  |  m1 |  m2 |  m3 |  m4 |
+-----+-----+-----+-----+-----+-----+
*/

/*
9.7. Server Identifier

This option is used in DHCPOFFER and DHCPREQUEST messages, and may
optionally be included in the DHCPACK and DHCPNAK messages.  DHCP
servers include this option in the DHCPOFFER in order to allow the
client to distinguish between lease offers.  DHCP clients use the
contents of the 'server identifier' field as the destination address
for any DHCP messages unicast to the DHCP server.  DHCP clients also
indicate which of several lease offers is being accepted by including
this option in a DHCPREQUEST message.

The identifier is the IP address of the selected server.

The code for this option is 54, and its length is 4.

 Code   Len            Address
 +-----+-----+-----+-----+-----+-----+
 |  54 |  4  |  a1 |  a2 |  a3 |  a4 |
 +-----+-----+-----+-----+-----+-----+
*/

/*
9.2. IP Address Lease Time

This option is used in a client request (DHCPDISCOVER or DHCPREQUEST)
to allow the client to request a lease time for the IP address.  In a
server reply (DHCPOFFER), a DHCP server uses this option to specify
the lease time it is willing to offer.

 The time is in units of seconds, and is specified as a 32-bit
 unsigned integer.

 The code for this option is 51, and its length is 4.

 Code   Len         Lease Time
 +-----+-----+-----+-----+-----+-----+
 |  51 |  4  |  t1 |  t2 |  t3 |  t4 |
 +-----+-----+-----+-----+-----+-----+
*/

/*
3.5. Router Option

The router option specifies a list of IP addresses for routers on the
client's subnet.  Routers SHOULD be listed in order of preference.

The code for the router option is 3.  The minimum length for the
router option is 4 octets, and the length MUST always be a multiple
of 4.

Code   Len         Address 1               Address 2
+-----+-----+-----+-----+-----+-----+-----+-----+--
|  3  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
+-----+-----+-----+-----+-----+-----+-----+-----+--
*/
/*
*/