/*
  *CHECK [ADDR_INFO_TERM]
  * How do we know if we need htons or not? dont all bits require an order check?
  * check source port
  * check stuff to free
  * why do we need checksum for both IP and TCP?
  *free addr info term?
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h> 
#include <netinet/ip.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include "read_json.h"

#define TCP_FLAG_LEN 8

/* 
  *this class is responsible for compression detection with an uncooperative server* 
  *uncooperative denotes inability to obtain control of the server (but its still responsive)*
*/

// we have two different checksums: One for our UDP packets and the other for the TCP connection.

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t udp4_checksum(struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen){
  char buf[];
  char *ptr;
  int chksumlen = 0;
  int i;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);

  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);

  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;

  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);

  // Copy UDP length to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP source port to buf (16 bits)
  memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
  ptr += sizeof (udphdr.source);
  chksumlen += sizeof (udphdr.source);

  // Copy UDP destination port to buf (16 bits)
  memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
  ptr += sizeof (udphdr.dest);
  chksumlen += sizeof (udphdr.dest);

  // Copy UDP length again to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  // Copy payload to buf
  memcpy (ptr, payload, payloadlen);
  ptr += payloadlen;
  chksumlen += payloadlen;

  // Pad to the next 16-bit boundary
  for (i=0; i<payloadlen%2; i++, ptr++) {
    *ptr = 0;
    ptr++;
    chksumlen++;
  }

  return checksum((uint16_t *)buf,chksumlen);
}


// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t checksum(uint16_t *addr, int len){
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while(count > 1){
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if(count > 0) {
    sum += *(uint8_t *)addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

// Allocate memory for an array of chars.
char* allocate_strmem(int len){
  void *tmp;

  if (len <= 0) {
    fprintf(stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit(EXIT_FAILURE);
  }

  tmp = (char *)malloc(len * sizeof (char));
  if (tmp != NULL){
    memset(tmp, 0, len * sizeof (char));
    return(tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit(EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int* allocate_intmem(int len){
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n",len);
    exit(EXIT_FAILURE);
  }

  tmp = (int *)malloc(len * sizeof(int));
  if(tmp != NULL){
    memset (tmp, 0, len * sizeof(int));
    return (tmp);
  } else{
    fprintf(stderr,"ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit(EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *allocate_ustrmem(int len){
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}


int main(int argc, char **argv){

  /* 
    * unlike our server_cooperative/client_cooperative  we are using raw sockets for deeper control over the
      packet data specifications (layers and payload)
  */

  if (argc != 2){
      fprintf (stderr, "ERROR: Too few or many arguments.\n");
      exit (EXIT_FAILURE);
  }

  //read JSON file to obtain data (TTL will be our main variable)
  struct json packet_info;
  char buff[1000] = {0};
  read_json(&, argv[1], buff);


  // initialize socket connection with the start of the client
  int sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if (sockfd == -1){ 
    fprintf (stderr, "ERROR: socket creation failed.\n");
    exit (EXIT_FAILURE);
  } 
  else
      printf("Socket successfully created..\n");


  // get our ethernet interface and search for its mac addresses
  struct ifreq eth_data;
  memset(&eth_data,0,sizeof(struct ifreq));
  strcpy(eth_data.ifr_name,"eth0");
  if(ioctl(sockfd, SIOCGIFHWADDR, &eth_data) != 1){
    fprintf (stderr, "ERROR: socket creation failed.\n");
    exit (EXIT_FAILURE);
  }

  close(sockfd);

  // take in the first 6 mac addresses that show up
  uint8_t* mac_addr_src;
  mac_addr_src= allocate_ustrmem(6);
  memcpy(mac_addr_src,ifr.ifr_hwaddr.sa_data,6);

  // find mac address that matches our interface frame
  struct sockaddr_ll send_to_attr;
  memset(&send_to_attr, 0,sizeof(struct sockaddr_ll));
  send_to_attr.sll_ifindex = if_nametoindex(eth_data.ifr_name);

  if (send_to_attr.sll_ifindex == 0){
    fprintf (stderr, "ERROR: unable to find index from interface name.\n");
    exit (EXIT_FAILURE);
  }

  // Set mac destination address
  mac_addr_dest;
  mac_addr_dest=allocate_ustrmem(6);
  memset(&mac_addr_dest,0xff,6);


  // prepare source and destination addresses
  char* src_ip, dest_ip;
  src_ip = allocate_strmem(INET_ADDRSTRLEN);
  dest_ip = allocate_strmem(INET_ADDRSTRLEN);
  strcpy(src_ip, "1.2.3.4"); 
  strcpy(dest_ip, packet_info.server_ip);


  // get our client and supportive server information for data exchange
  int recv_info, exec;
  struct addrinfo addr_info_init, addr_info_term;
  memset(&addr_info_init,0,sizeof(struct addrinfo));
  addr_info_init.ai_family = AF_INET;
  addr_info_init.ai_socktype = SOCK_STREAM;
  addr_info_init.ai_flags = hints.ai_flags | AI_CANONNAME;

  if ((recv_info= getaddrinfo(dest_ip,NULL,&addr_info_init,&addr_info_term))!= 0){
      fprintf(stderr, "ERROR: Failed to recieve address information.\n");
      exit(EXIT_FAILURE);
  }

  // transorm destination address from binary to string
  struct sockaddr_in* ip_v4;
  ip_v4=(struct sockaddr_in *)addr_info_term->ai_addr;

  if(inet_ntop(AF_INET,(void *)&(ip_v4->sin_addr,dest_ip),INET_ADDRSTRLEN) == NULL){
      fprintf(stderr, "ERROR: Failed to convert destination address information to string.\n");
      exit(EXIT_FAILURE);
  }


  // continue fillout of ethernet frame information
  eth_data.sll_family = AF_PACKET; // this enables the to control protocol selection
  eth_data.sll_protocol = htons(ETH_P_IP); // select protocol
  memcpy(eth_data.sll_addr,mac_addr_dest,6); // ethernet destination address
  eth_data.sll_halen=6; // ethernet address length


  // prepare ip header for TCP manually
  int tcp_payload_length =packet_info.payload_sz;
  uint8_t* tcp_payload;
  tcp_payload= allocate_ustrmem(tcp_payload_length);
  int* ip_flags;
  struct ip ip_header;
  ip_header.ip_hl =IP4_HDRLEN/sizeof(uint32_t);
  ip_header.ip_v =4;
  ip_header.ip_tos = 0;
  ip_header.ip_len = htons(IP4_HDRLEN + TCP_HDRLEN + tcp_payload_length); // htons makes sure we are using little-endian byte order
  ip_header.ip_id= htons(0);

  ip_flags[0]=0; // static unused bit
  ip_flags[1]=0; // DF flag
  ip_flags[2]=0; // MF flag
  ip_flags[3]=0; // fragment offset

  // position flag results into corresponding sectors in ip_off by shifting the bits to correct order 
  ip_header.ip_off = htons((ip_flags[0]<<15)+(ip_flags[1]<<14)+(ip_flags[2] << 13)+ip_flags[3]); 

  ip_header.ip_ttl =packet_info.TTL;
  ip_header.ip_p = IPPROTO_TCP;


  /* before setting our src and dest addresses in our IP header, we should convert the string representations 
     into the IP address format of type IPv4 (dot notation)
  */

  if ((exec = inet_pton(AF_INET, src_ip, &(ip_header.ip_src)))!= 1){
    fprintf(stderr, "Failed to convert string to source IP address.\nError message: %s",strerror(exec));
    exit (EXIT_FAILURE);
  }

  if ((exec = inet_pton(AF_INET, dst_ip, &(ip_header.ip_dst))) != 1){
    fprintf(stderr, "Failed to convert string to destination IP address.\nError message: %s",strerror(exec));
    exit (EXIT_FAILURE);
  }

  ip_header.ip_sum = 0;
  ip_header.ip_sum = checksum((uint16_t *)&ip_header,IP4_HDRLEN);


  // TCP|SYN manual packet setup

  struct tcphdr tcp_header;

  tcp_header.th_sport= htons(60); // source port
  tcp_header.th_dport= htons(packet_info.dest_prt_tcp_head); // destination port
  tcp_header.th_seq= htonl(1); //sequence number 
  tcp_header.th_ack= htonl(0); //ACK response number
  tcp_header.th_off=TCP_HDRLEN/4; //divide by 4 because TCP header length is made from offsets of multiples of 4

  int* tcp_flags;

  tcp_flags=allocate_intmem(TCP_FLAG_LEN);
  tcp_flags[1]=1; //this means the only flag we have on is to define its identity as a SYN packet

  tcp_header.th_flags= 0;
  int i;
  for (i=0; i<TCP_FLAG_LEN; i++) {
    tcp_header.th_flags+=(tcp_flags[i]<<i); //place flags in respective bit order.
  }

  tcp_header.th_win= htons(32767); //set window size
  tcp_header.th_urp= htons(0); //urgent pointer
  tcp_header.th_sum= 0; //TCP checksum


  uint8_t* tcp_frame;
  tcp_frame= allocate_ustrmem(IP_MAXPACKET);
  int tcp_frame_length;
  tcp_frame_length = IP4_HDRLEN + TCP_HDRLEN + tcp_payload_length;

  // using pointer arithmetic declare space for each layer of the ethernet frame
  memcpy(tcp_frame,&tcp_header, IP4_HDRLEN); //set IP header size
  memcpy(tcp_frame+IP4_HDRLEN, &tcp_header, TCP_HDRLEN); //set TCP header size
  memcpy(tcp_frame+IP4_HDRLEN+TCP_HDRLEN,tcp_payload,tcp_payload_length); // set payload size

  // connect to socket to send SYN packet  
  if ((sockfd = socket(PF_PACKET,SOCK_STREAM,htons(ETH_P_ALL)))!= 1){
    fprintf(stderr, "ERROR: unable to establish socket connection.\n");
    exit(EXIT_FAILURE);
  }

  // Send ethernet frame to socket.
  if ((bytes = sendto(sockfd, tcp_frame,tcp_frame_length, 0,(struct sockaddr *)&send_to_attr, sizeof(send_to_attr))) <= 0) {
    fprintf(stderr, "ERROR: unable to send TCP ethernet frame.\n");
    exit(EXIT_FAILURE);
  }

  close (sockfd);

  // start with a single head SYN packet (to port x) --> this will trigger RST packets to be sent from the server

  // follow it with a train of UDP packets (ignore ICMP packets sent back in this part)

  // end with a single tail SYN packet (to port y) --> this will trigger RST packets to be sent from the server

  //calculate the difference between arrival time of the two RST packets for compression analysis (loss may occur)
  
  
  return 0;
}