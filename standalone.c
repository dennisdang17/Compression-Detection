#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <string.h>           
#include <netdb.h>            
#include <sys/types.h>       
#include <sys/socket.h>    
#include <netinet/in.h>
#include <arpa/inet.h>      
#include <errno.h>
#include <time.h> 
#include <ctype.h>
#include <json-c/json.h>
#include <pcap.h>
#include <netinet/tcp.h> 
#include <netinet/udp.h>
#include <sys/ioctl.h> 
#include <netinet/ip.h>
#include <net/if.h>

#define BUF_SIZE 2000

//this function creates unique checksum for the IP headers
uint16_t checksum(uint16_t *addr, int len)
{
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;

    while(count > 1){
        sum += *(addr++); 
        count -= 2;
    }

    if(count > 0) {
        sum += *(uint8_t *)addr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    answer = ~sum;
    return (answer);
}

//this function creates unique checksum for the UDP headers
uint16_t udp4_checksum(struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
{
    char buf[IP_MAXPACKET];
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

    return checksum ((uint16_t *) buf, chksumlen);
}

//this function creates unique checksum for the TCP headers
uint16_t tcp4_checksum (struct ip iphdr, struct tcphdr tcphdr)
{
    uint16_t svalue;
    char buf[IP_MAXPACKET], cvalue;
    char *ptr;
    int chksumlen = 0;

    // ptr points to beginning of buffer buf
    ptr = &buf[0];

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

    // Copy TCP length to buf (16 bits)
    svalue = htons (sizeof (tcphdr));
    memcpy (ptr, &svalue, sizeof (svalue));
    ptr += sizeof (svalue);
    chksumlen += sizeof (svalue);

    // Copy TCP source port to buf (16 bits)
    memcpy (ptr, &tcphdr.th_sport, sizeof (tcphdr.th_sport));
    ptr += sizeof (tcphdr.th_sport);
    chksumlen += sizeof (tcphdr.th_sport);

    // Copy TCP destination port to buf (16 bits)
    memcpy (ptr, &tcphdr.th_dport, sizeof (tcphdr.th_dport));
    ptr += sizeof (tcphdr.th_dport);
    chksumlen += sizeof (tcphdr.th_dport);

    // Copy sequence number to buf (32 bits)
    memcpy (ptr, &tcphdr.th_seq, sizeof (tcphdr.th_seq));
    ptr += sizeof (tcphdr.th_seq);
    chksumlen += sizeof (tcphdr.th_seq);

    // Copy acknowledgement number to buf (32 bits)
    memcpy (ptr, &tcphdr.th_ack, sizeof (tcphdr.th_ack));
    ptr += sizeof (tcphdr.th_ack);
    chksumlen += sizeof (tcphdr.th_ack);

    // Copy data offset to buf (4 bits) and
    // copy reserved bits to buf (4 bits)
    cvalue = (tcphdr.th_off << 4) + tcphdr.th_x2;
    memcpy (ptr, &cvalue, sizeof (cvalue));
    ptr += sizeof (cvalue);
    chksumlen += sizeof (cvalue);

    // Copy TCP flags to buf (8 bits)
    memcpy (ptr, &tcphdr.th_flags, sizeof (tcphdr.th_flags));
    ptr += sizeof (tcphdr.th_flags);
    chksumlen += sizeof (tcphdr.th_flags);

    // Copy TCP window size to buf (16 bits)
    memcpy (ptr, &tcphdr.th_win, sizeof (tcphdr.th_win));
    ptr += sizeof (tcphdr.th_win);
    chksumlen += sizeof (tcphdr.th_win);
    return checksum ((uint16_t *) buf, chksumlen);
}

// Allocate memory for an array of chars
char * allocate_strmem(int len)
{
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

// Allocate memory for an array of unsigned chars.
uint8_t * allocate_ustrmem(int len)
{
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

// Allocate memory for an array of ints.
int * allocate_intmem(int len)
{
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

int main(int argc, char **argv)
{
    FILE * fp;
    char buffer[BUF_SIZE];
    struct sockaddr_in server_address, client_address;
    struct json_object *parsed_json, *Server_IP_Address, *Source_Port_Number_UDP, *Destination_Port_Number_UDP,
    *Destination_Port_Number_TCP_Head, *Destination_Port_Number_TCP_Tail, *Port_Number_TCP, 
    *Size_UDP_Payload, *Inter_Measurement_Time, *Number_UDP_Packets, *TTL_UDP_Packets;
    enum{UDP_HDRLEN=8, ICMP_HDRLEN=8, TCP_HDRLEN=20, IP4_HDRLEN=20};

    //Check for proper usage
    if (argv[1] == NULL)
    {
        printf("ERROR!\nProper ussage ./'name of executable' 'my_config_file'.json\n");
        return EXIT_FAILURE;
    }

    //Open config file
    fp = fopen(argv[1],"r"); //opens the file myconfig.json
    if(fp == NULL)
    {
        printf("ERROR OPENNING FILE!\n"); //catch null pointer
        return EXIT_FAILURE;
    }
    printf("Parsing...\n");
    fread(buffer, BUF_SIZE, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer); //parse JSON file's contents and converts them into a JSON object

    //Store parsed data into variables
    json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
    json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_UDP", &Destination_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
    json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
    json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
    json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
    json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
    json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);
    printf("Parsing Successful\n");

    printf("(Testing) Preparing to send packets!\n");
    printf("Sending...\n");
    
    int i, status, sd, *ip_flags, *tcp_flags;
    const int on = 1;
    char *interface, *target, *src_ip, *dst_ip;
    struct ip iphdr;
    struct tcphdr tcphdr;
    uint8_t *tcp_pkt_hd, *udp_pkt, *tcp_pkt_tl, *udp_pkt_2;
    struct addrinfo hints, *res;
    struct sockaddr_in *ipv4, sin;
    struct ifreq ifr;
    void *tmp;

    // Allocate memory for various arrays.
    tcp_pkt_hd = allocate_ustrmem (IP_MAXPACKET);
    tcp_pkt_tl = allocate_ustrmem (IP_MAXPACKET);
    interface = allocate_strmem (40);
    target = allocate_strmem (40);
    src_ip = allocate_strmem (INET_ADDRSTRLEN);
    dst_ip = allocate_strmem (INET_ADDRSTRLEN);
    ip_flags = allocate_intmem (4);
    tcp_flags = allocate_intmem (8);

    // Interface to send packet through.
    strcpy (interface, "enp0s3"); //eth0 does not compile
    sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    
    // Submit request for a socket descriptor to look up interface.
    if (sd < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl() ");
        exit (EXIT_FAILURE);
    }

    // Use ioctl() to look up interface name and get its MAC address.
    memset (&ifr, 0, sizeof (ifr));
    snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
    if (ioctl (sd, SIOCGIFINDEX, &ifr) < 0) {
        perror ("ioctl() failed to find interface ");
        return (EXIT_FAILURE);
    }
    close (sd);

    // Source IPv4 address: you need to fill this out
    strcpy (src_ip, "10.0.0.249");

    // Destination URL or IPv4 address: you need to fill this out
    strcpy (target, json_object_get_string(Server_IP_Address));

    // Fill out hints for getaddrinfo().
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = hints.ai_flags | AI_CANONNAME;
    status = getaddrinfo (target, NULL, &hints, &res);
    
    // Resolve target using getaddrinfo().
    if (status != 0) {
        fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
        exit (EXIT_FAILURE);
    }
    ipv4 = (struct sockaddr_in *) res->ai_addr;
    tmp = &(ipv4->sin_addr);
    if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
        status = errno;
        fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
        exit (EXIT_FAILURE);
    }
    freeaddrinfo (res);

    // IPv4 header
    // IPv4 header length (4 bits): Number of 32-bit words in header = 5
    iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);

    // Internet Protocol version (4 bits): IPv4
    iphdr.ip_v = 4;

    // Type of service (8 bits)
    iphdr.ip_tos = 0;

    // Total length of datagram (16 bits): IP header + TCP header
    iphdr.ip_len = htons (IP4_HDRLEN + TCP_HDRLEN);

    // ID sequence number (16 bits): unused, since single datagram
    iphdr.ip_id = htons (0);

    // Zero (1 bit)
    ip_flags[0] = 0;

    // Do not fragment flag (1 bit)
    ip_flags[1] = 0;

    // More fragments following flag (1 bit)
    ip_flags[2] = 0;

    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;

    iphdr.ip_off = htons ((ip_flags[0] << 15)
    + (ip_flags[1] << 14)
    + (ip_flags[2] << 13)
    +  ip_flags[3]);

    // Time-to-Live (8 bits): default to maximum value
    iphdr.ip_ttl = 255;

    // Transport layer protocol (8 bits): 6 for TCP
    iphdr.ip_p = IPPROTO_TCP;
    status = inet_pton (AF_INET, src_ip, &(iphdr.ip_src));
    
    // Source IPv4 address (32 bits)
    if (status != 1) {
        fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
        exit (EXIT_FAILURE);
    }

    status = inet_pton (AF_INET, dst_ip, &(iphdr.ip_dst));
    // Destination IPv4 address (32 bits)
    if (status != 1) {
        fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
        exit (EXIT_FAILURE);
    }

    // IPv4 header checksum (16 bits): set to 0 when calculating checksum
    iphdr.ip_sum = 0;
    iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

    // TCP header
    // Source port number (16 bits)
    tcphdr.th_sport = htons (8080);

    // Destination port number (16 bits)
    tcphdr.th_dport = htons (json_object_get_int(Destination_Port_Number_TCP_Head));

    // Sequence number (32 bits)
    tcphdr.th_seq = htonl (0);

    // Acknowledgement number (32 bits): 0 in first packet of SYN/ACK process
    tcphdr.th_ack = htonl (0);

    // FIN flag (1 bit)
    tcp_flags[0] = 0;
    // SYN flag (1 bit): set to 1
    tcp_flags[1] = 1;
    // RST flag (1 bit)
    tcp_flags[2] = 1;
    // PSH flag (1 bit)
    tcp_flags[3] = 0;
    // ACK flag (1 bit)
    tcp_flags[4] = 0;

    tcphdr.th_flags = 0;

    for (i=0; i<8; i++) {
        tcphdr.th_flags += (tcp_flags[i] << i);
    }

    // TCP checksum (16 bits)
    tcphdr.th_sum = tcp4_checksum (iphdr, tcphdr);

    // TCP checksum (16 bits)
    tcphdr.th_sum = tcp4_checksum (iphdr, tcphdr);

    // First part is an IPv4 header.
    memcpy (tcp_pkt_tl, &iphdr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((tcp_pkt_tl + IP4_HDRLEN), &tcphdr, TCP_HDRLEN * sizeof (uint8_t));
    
    memset (&sin, 0, sizeof (struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    // Submit request for a raw socket descriptor.
    if (sd < 0) {
        perror ("socket() failed ");
        exit (EXIT_FAILURE);
    }

    // Set flag so socket expects us to provide IPv4 header.
    if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) {
        perror ("setsockopt() failed to set IP_HDRINCL ");
        exit (EXIT_FAILURE);
    }

    // Bind socket to interface index.
    if (setsockopt (sd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
        perror ("setsockopt() failed to bind to interface ");
        exit (EXIT_FAILURE);
    }

    struct udphdr udphdr;

    uint8_t * data = allocate_ustrmem (json_object_get_int(Size_UDP_Payload));
    
    // UDP data
    int datalen = json_object_get_int(Size_UDP_Payload);
    memset (data, 0, json_object_get_int(Size_UDP_Payload));
    // Transport layer protocol (8 bits): 17 for UDP
    iphdr.ip_p = IPPROTO_UDP;
    iphdr.ip_ttl = json_object_get_int(TTL_UDP_Packets);


    // IPv4 header checksum (16 bits): set to 0 when calculating checksum
    iphdr.ip_sum = 0;
    iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

    // UDP header

    // Source port number (16 bits): pick a number
    udphdr.source = htons (4950);

    // Destination port number (16 bits): pick a number
    udphdr.dest = htons (9999);
    udphdr.len = htons (UDP_HDRLEN + datalen);

    // UDP checksum (16 bits)
    udphdr.check = udp4_checksum (iphdr, udphdr, data, datalen);
    int frame_length = IP4_HDRLEN + UDP_HDRLEN + datalen;
    udp_pkt = allocate_ustrmem (IP_MAXPACKET);
    udp_pkt_2 = allocate_ustrmem (IP_MAXPACKET);

    // IPv4 header
    memcpy (udp_pkt, &iphdr, IP4_HDRLEN * sizeof (uint8_t));

    // UDP header
    memcpy (udp_pkt + IP4_HDRLEN, &udphdr, UDP_HDRLEN);

    // Send ethernet frame to socket.
    if (sendto (sd, tcp_pkt_hd, IP4_HDRLEN + TCP_HDRLEN, 0, (struct sockaddr *) &sin, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        exit (EXIT_FAILURE);
    }

    printf("(Testing) Success: All required packets have been sent!\n");
    close (sd);
    return (EXIT_SUCCESS);
}