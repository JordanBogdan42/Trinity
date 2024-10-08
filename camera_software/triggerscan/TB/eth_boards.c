#include <pci/pci.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "bitfile.h"
#include "anyio.h"
#include "eth_boards.h"
#include "eeprom_remote.h"
#include "lbp16.h"
#include "eeprom.h"

#define USE_RAW_SOCKETS
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data

extern board_t boards[MAX_BOARDS];
extern int boards_count;

int sd;
socklen_t len;
struct sockaddr_in dst_addr, src_addr;

// eth access functions

#ifdef USE_RAW_SOCKETS
// Checksum function
uint16_t
checksum (uint16_t *addr, int len)
{
  int nleft = len;
  int sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= sizeof (uint16_t);
  }

  if (nleft == 1) {
    *(uint8_t *) (&answer) = *(uint8_t *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t
udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
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

  // Pad to the next 16-bit boundary
  for (i=0; i<payloadlen%2; i++, ptr++) {
    *ptr = 0;
    ptr++;
    chksumlen++;
  }

  return checksum ((uint16_t *) buf, chksumlen);
}

#endif

int eth_socket_send_packet(void *packet, int size) {
#ifdef USE_RAW_SOCKETS
    struct ip iphdr;
    struct udphdr udphdr;
    int status;
    u8 ip_flags[4];
    char src_ip[] = "192.168.1.10";
    u8 send_packet[1500];

    iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);
    // Internet Protocol version (4 bits): IPv4
    iphdr.ip_v = 4;
    // Type of service (8 bits)
    iphdr.ip_tos = 0;
    // Total length of datagram (16 bits): IP header + UDP header + datalen
    iphdr.ip_len = htons (IP4_HDRLEN + UDP_HDRLEN + size);
    // ID sequence number (16 bits): unused, since single datagram
    iphdr.ip_id = htons (0);
    // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram
    // Zero (1 bit)
    ip_flags[0] = 0;
    // Do not fragment flag (1 bit)
    ip_flags[1] = 0;
    // More fragments following flag (1 bit)
    ip_flags[2] = 0;
    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;
    iphdr.ip_off = htons ((ip_flags[0] << 15) + (ip_flags[1] << 14) + (ip_flags[2] << 13) + ip_flags[3]);
    // Time-to-Live (8 bits): default to maximum value
    iphdr.ip_ttl = 255;
    // Transport layer protocol (8 bits): 17 for UDP
    iphdr.ip_p = IPPROTO_UDP;

    // Source IPv4 address (32 bits)
    if ((status = inet_pton (AF_INET, src_ip, &(iphdr.ip_src))) != 1) {
        fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
        exit (EXIT_FAILURE);
    }
    iphdr.ip_dst = dst_addr.sin_addr;
    // IPv4 header checksum (16 bits): set to 0 when calculating checksum
    iphdr.ip_sum = 0;
    iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

    // UDP header
    // Source port number (16 bits): pick a number
    udphdr.source = htons (LBP16_UDP_PORT);
    // Destination port number (16 bits): pick a number
    udphdr.dest = htons (LBP16_UDP_PORT);
    // Length of UDP datagram (16 bits): UDP header + UDP data
    udphdr.len = htons (UDP_HDRLEN + size);
    // UDP checksum (16 bits)
    udphdr.check = udp4_checksum (iphdr, udphdr, packet, size);

    // Prepare packet.
    // First part is an IPv4 header.
    memcpy (send_packet, &iphdr, IP4_HDRLEN * sizeof (uint8_t));
    // Next part of packet is upper layer protocol header.
    memcpy ((send_packet + IP4_HDRLEN), &udphdr, UDP_HDRLEN * sizeof (uint8_t));
    // Finally, add the UDP data.
    memcpy (send_packet + IP4_HDRLEN + UDP_HDRLEN, packet, size * sizeof (uint8_t));

    return sendto(sd, (char*) send_packet, IP4_HDRLEN + UDP_HDRLEN + size, 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
#endif
    return sendto(sd, (char*) packet, size, 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
}

int eth_socket_recv_packet(void *buffer, int size) {
#ifdef USE_RAW_SOCKETS
    u8 recv_packet[1500];
    int recv;

    recv = recvfrom(sd, (char*) recv_packet, IP4_HDRLEN + UDP_HDRLEN + size, 0, (struct sockaddr *) &src_addr, &len);
    memcpy(buffer, &recv_packet[IP4_HDRLEN + UDP_HDRLEN], size);
    return recv;
#else
    return recvfrom(sd, (char*) buffer, size, 0, (struct sockaddr *) &src_addr, &len);
#endif
}

static void eth_socket_nonblocking() {
    int val = fcntl(sd, F_GETFL);
    val = val | O_NONBLOCK;
    fcntl(sd, F_SETFL, val);
}

static void eth_socket_blocking() {
    int val = fcntl(sd, F_GETFL);
    val = val & ~O_NONBLOCK;
    fcntl(sd, F_SETFL, val);
}

void eth_socket_set_dest_ip(char *addr_name) {
    dst_addr.sin_addr.s_addr = inet_addr(addr_name);
}

static char *eth_socket_get_src_ip() {
    return inet_ntoa(src_addr.sin_addr);
}

int lbp16_read(u16 cmd, u32 addr, void *buffer, int size) {
    lbp16_cmd_addr packet;
    int send, recv;
    u8 local_buff[size];

    LBP16_INIT_PACKET4(packet, cmd, addr);
    if (LBP16_SENDRECV_DEBUG)
        printf("SEND: %02X %02X %02X %02X | REQUEST %d bytes\n", packet.cmd_hi, packet.cmd_lo, packet.addr_hi, packet.addr_lo, size);
    send = eth_socket_send_packet(&packet, sizeof(packet));
    recv = eth_socket_recv_packet(&local_buff, sizeof(local_buff));
    if (LBP16_SENDRECV_DEBUG)
        printf("RECV: %d bytes\n", recv);
    memcpy(buffer, local_buff, size);

    return 0;
}

int lbp16_write(u16 cmd, u32 addr, void *buffer, int size) {
    static struct {
        lbp16_cmd_addr wr_packet;
        u8 tmp_buffer[127*8];
    } packet;
    int send;

    LBP16_INIT_PACKET4(packet.wr_packet, cmd, addr);
    memcpy(&packet.tmp_buffer, buffer, size);
    if (LBP16_SENDRECV_DEBUG)
        printf("SEND: %02X %02X %02X %02X | WRITE %d bytes\n", packet.wr_packet.cmd_hi, packet.wr_packet.cmd_lo,
          packet.wr_packet.addr_hi, packet.wr_packet.addr_lo, size);
    send = eth_socket_send_packet(&packet, sizeof(lbp16_cmd_addr) + size);

    return 0;
}

static int eth_read(llio_t *this, u32 addr, void *buffer, int size) {
    if ((size/4) > LBP16_MAX_PACKET_DATA_SIZE) {
        printf("ERROR: LBP16: Requested %d units to read, but protocol supports up to %d units to be read per packet\n", size/4, LBP16_MAX_PACKET_DATA_SIZE);
        return -1;
    }

    return lbp16_read(CMD_READ_HOSTMOT2_ADDR32_INCR(size/4), addr, buffer, size);
}

static int eth_write(llio_t *this, u32 addr, void *buffer, int size) {
    if ((size/4) > LBP16_MAX_PACKET_DATA_SIZE) {
        printf("ERROR: LBP16: Requested %d units to write, but protocol supports up to %d units to be write per packet\n", size/4, LBP16_MAX_PACKET_DATA_SIZE);
        return -1;
    }

    return lbp16_write(CMD_WRITE_HOSTMOT2_ADDR32_INCR(size/4), addr, buffer, size);
}

static int eth_board_open(board_t *board) {
    eth_socket_set_dest_ip(board->dev_addr);
    eeprom_init(&(board->llio));
    lbp16_read(CMD_READ_FLASH_IDROM, FLASH_ID_REG, &(board->flash_id), 4);
    eeprom_prepare_boot_block(board->flash_id);
    if (board->fallback_support == 1)
        board->flash_start_address = eeprom_calc_user_space(board->flash_id);
    else
        board->flash_start_address = 0;
    return 0;
}

static int eth_board_close(board_t *board) {
    return 0;
}

static int eth_scan_one_addr(board_access_t *access) {

    lbp16_cmd_addr packet, packet2;
    int send = 0, recv = 0, ret = 0;
    u32 cookie;

    LBP16_INIT_PACKET4(packet, CMD_READ_HM2_COOKIE, HM2_COOKIE_REG);
    send = eth_socket_send_packet(&packet, sizeof(packet));
    sleep_ns(10*1000*1000);
    recv = eth_socket_recv_packet(&cookie, sizeof(cookie));

    if ((recv > 0) && (cookie == HM2_COOKIE)) {
        char buff[20];
        board_t *board = &boards[boards_count];

        eth_socket_blocking();
        LBP16_INIT_PACKET4(packet2, CMD_READ_BOARD_INFO_ADDR16_INCR(8), 0);
        memset(buff, 0, sizeof(buff));
        send = eth_socket_send_packet(&packet2, sizeof(packet2));
        recv = eth_socket_recv_packet(&buff, sizeof(buff));

        if (strncmp(buff, "7I80DB-16", 9) == 0) {
            board->type = BOARD_ETH;
            strncpy(board->dev_addr, eth_socket_get_src_ip(), 16);
            strncpy(board->llio.board_name, buff, 16);
            board->llio.num_ioport_connectors = 4;
            board->llio.pins_per_connector = 17;
            board->llio.ioport_connector_name[0] = "J2";
            board->llio.ioport_connector_name[1] = "J3";
            board->llio.ioport_connector_name[2] = "J4";
            board->llio.ioport_connector_name[3] = "J5";
            board->llio.fpga_part_number = "6slx16ftg256";
            board->llio.num_leds = 4;
            board->llio.read = &eth_read;
            board->llio.write = &eth_write;
            board->llio.write_flash = &remote_write_flash;
            board->llio.verify_flash = &remote_verify_flash;
            board->llio.private = board;

            board->open = &eth_board_open;
            board->close = &eth_board_close;
            board->print_info = &eth_print_info;
            board->flash = BOARD_FLASH_REMOTE;
            board->fallback_support = 1;
            board->llio.verbose = access->verbose;
        } else if (strncmp(buff, "7I80DB-25", 9) == 0) {
            board->type = BOARD_ETH;
            strncpy(board->dev_addr, eth_socket_get_src_ip(), 16);
            strncpy(board->llio.board_name, buff, 16);
            board->llio.num_ioport_connectors = 4;
            board->llio.pins_per_connector = 17;
            board->llio.ioport_connector_name[0] = "J2";
            board->llio.ioport_connector_name[1] = "J3";
            board->llio.ioport_connector_name[2] = "J4";
            board->llio.ioport_connector_name[3] = "J5";
            board->llio.fpga_part_number = "6slx25ftg256";
            board->llio.num_leds = 4;
            board->llio.read = &eth_read;
            board->llio.write = &eth_write;
            board->llio.write_flash = &remote_write_flash;
            board->llio.verify_flash = &remote_verify_flash;
            board->llio.private = board;

            board->open = &eth_board_open;
            board->close = &eth_board_close;
            board->print_info = &eth_print_info;
            board->flash = BOARD_FLASH_REMOTE;
            board->fallback_support = 1;
            board->llio.verbose = access->verbose;
        } else if (strncmp(buff, "7I80HD-16", 9) == 0) {
            board->type = BOARD_ETH;
            strncpy(board->dev_addr, eth_socket_get_src_ip(), 16);
            strncpy(board->llio.board_name, buff, 16);
            board->llio.num_ioport_connectors = 3;
            board->llio.pins_per_connector = 24;
            board->llio.ioport_connector_name[0] = "P1";
            board->llio.ioport_connector_name[1] = "P2";
            board->llio.ioport_connector_name[2] = "P3";
            board->llio.fpga_part_number = "6slx16ftg256";
            board->llio.num_leds = 4;
            board->llio.read = &eth_read;
            board->llio.write = &eth_write;
            board->llio.write_flash = &remote_write_flash;
            board->llio.verify_flash = &remote_verify_flash;
            board->llio.private = board;

            board->open = &eth_board_open;
            board->close = &eth_board_close;
            board->print_info = &eth_print_info;
            board->flash = BOARD_FLASH_REMOTE;
            board->fallback_support = 1;
            board->llio.verbose = access->verbose;
        } else if (strncmp(buff, "7I80HD-25", 9) == 0) {
            board->type = BOARD_ETH;
            strncpy(board->dev_addr, eth_socket_get_src_ip(), 16);
            strncpy(board->llio.board_name, buff, 16);
            board->llio.num_ioport_connectors = 3;
            board->llio.pins_per_connector = 24;
            board->llio.ioport_connector_name[0] = "P1";
            board->llio.ioport_connector_name[1] = "P2";
            board->llio.ioport_connector_name[2] = "P3";
            board->llio.fpga_part_number = "6slx25ftg256";
            board->llio.num_leds = 4;
            board->llio.read = &eth_read;
            board->llio.write = &eth_write;
            board->llio.write_flash = &remote_write_flash;
            board->llio.verify_flash = &remote_verify_flash;
            board->llio.private = board;

            board->open = &eth_board_open;
            board->close = &eth_board_close;
            board->print_info = &eth_print_info;
            board->flash = BOARD_FLASH_REMOTE;
            board->fallback_support = 1;
            board->llio.verbose = access->verbose;
        } else if (strncmp(buff, "7I76E-16", 9) == 0) {
            board->type = BOARD_ETH;
            strncpy(board->dev_addr, eth_socket_get_src_ip(), 16);
            strncpy(board->llio.board_name, buff, 16);
            board->llio.num_ioport_connectors = 3;
            board->llio.pins_per_connector = 17;
            board->llio.ioport_connector_name[0] = "on-card";
            board->llio.ioport_connector_name[1] = "P1";
            board->llio.ioport_connector_name[2] = "P2";
            board->llio.fpga_part_number = "6slx16ftg256";
            board->llio.num_leds = 4;
            board->llio.read = &eth_read;
            board->llio.write = &eth_write;
            board->llio.write_flash = &remote_write_flash;
            board->llio.verify_flash = &remote_verify_flash;
            board->llio.private = board;

            board->open = &eth_board_open;
            board->close = &eth_board_close;
            board->print_info = &eth_print_info;
            board->flash = BOARD_FLASH_REMOTE;
            board->fallback_support = 1;
            board->llio.verbose = access->verbose;
        } else {
            printf("Unsupported ethernet device %s at %s\n", buff, eth_socket_get_src_ip());
            ret = -1;
        }
        boards_count++;

        eth_socket_nonblocking();
    }
    return ret;
}

// public functions

int eth_boards_init(board_access_t *access) {
// open socket
#ifdef USE_RAW_SOCKETS
    const int on = 1;

    sd = socket (AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) {
        perror ("setsockopt() failed to set IP_HDRINCL ");
        exit (EXIT_FAILURE);
    }
#else
    sd = socket (PF_INET, SOCK_DGRAM, 0);
#endif

    src_addr.sin_family = AF_INET;
    dst_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(LBP16_UDP_PORT);
    dst_addr.sin_port = htons(LBP16_UDP_PORT);
    len = sizeof(src_addr);
    return 0;
}

void eth_boards_cleanup(board_access_t *access) {
    int i;

    for (i = 0; i < boards_count; i++) {
        board_t *board = &boards[i];

        eeprom_cleanup(&(board->llio));
    }

    close(sd);
}

void eth_boards_scan(board_access_t *access) {
    char addr[16];
    int i;
    char *ptr;

    if (inet_pton(AF_INET, access->dev_addr, addr) != 1) {
        return;
    };

    eth_socket_nonblocking();

    if (access->address == 1) {
        eth_socket_set_dest_ip(access->dev_addr);
        eth_scan_one_addr(access);
    } else {
        strncpy(addr, access->dev_addr, 16);
        ptr = strrchr(addr, '.');
        *ptr = '\0';

        for (i = 1; i < 255; i++) {
            char addr_name[32];

            sprintf(addr_name, "%s.%d", addr, i);
            eth_socket_set_dest_ip(addr_name);
            eth_scan_one_addr(access);
        }
    }
    eth_socket_blocking();
}

void eth_print_info(board_t *board) {
    lbp16_cmd_addr packet;
    int i, j;
    u32 flash_id;
    char *mem_types[16] = {NULL, "registers", "memory", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "EEPROM", "flash"};
    char *mem_writeable[2] = {"RO", "RW"};
    char *acc_types[4] = {"8-bit", "16-bit", "32-bit", "64-bit"};
    char *led_debug_types[2] = {"Hostmot2", "eth debug"};
    char *boot_jumpers_types[4] = {"fixed "LBP16_HW_IP, "fixed from EEPROM", "from BOOTP", "INVALID"};
    lbp_mem_info_area mem_area;
    lbp_eth_eeprom_area eth_area;
    lbp_timers_area timers_area;
    lbp_status_area stat_area;
    lbp_info_area info_area;
    lbp16_cmd_addr cmds[LBP16_MEM_SPACE_COUNT];

    printf("\nETH device %s at ip=%s\n", board->llio.board_name, eth_socket_get_src_ip());
    if (board->llio.verbose == 0)
        return;
        
    printf("Board info:\n");
    if (board->flash_id > 0)
        printf("  Flash size: %s (id: 0x%02X)\n", eeprom_get_flash_type(board->flash_id), board->flash_id);
    printf("  Connectors count: %d\n", board->llio.num_ioport_connectors);
    printf("  Pins per connector: %d\n", board->llio.pins_per_connector);
    printf("  Connectors names:");
    for (i = 0; i < board->llio.num_ioport_connectors; i++)
        printf(" %s", board->llio.ioport_connector_name[i]);
    printf("\n");
    printf("  FPGA type: %s\n", board->llio.fpga_part_number);
    printf("  Number of leds: %d\n", board->llio.num_leds);

    LBP16_INIT_PACKET4(cmds[0], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_HM2, sizeof(mem_area)/2), 0);
    LBP16_INIT_PACKET4(cmds[1], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_ETH_CHIP, sizeof(mem_area)/2), 0);
    LBP16_INIT_PACKET4(cmds[2], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_ETH_EEPROM, sizeof(mem_area)/2), 0);
    LBP16_INIT_PACKET4(cmds[3], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_FPGA_FLASH, sizeof(mem_area)/2), 0);
    LBP16_INIT_PACKET4(cmds[4], 0, 0);
    LBP16_INIT_PACKET4(cmds[5], 0, 0);
    LBP16_INIT_PACKET4(cmds[6], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_COMM_CTRL, sizeof(mem_area)/2), 0);
    LBP16_INIT_PACKET4(cmds[7], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_BOARD_INFO, sizeof(mem_area)/2), 0);

    LBP16_INIT_PACKET4(packet, CMD_READ_ETH_EEPROM_ADDR16_INCR(sizeof(eth_area)/2), 0);
    memset(&eth_area, 0, sizeof(eth_area));
    eth_socket_send_packet(&packet, sizeof(packet));
    eth_socket_recv_packet(&eth_area, sizeof(eth_area));

    LBP16_INIT_PACKET4(packet, CMD_READ_COMM_CTRL_ADDR16_INCR(sizeof(stat_area)/2), 0);
    memset(&stat_area, 0, sizeof(stat_area));
    eth_socket_send_packet(&packet, sizeof(packet));
    eth_socket_recv_packet(&stat_area, sizeof(stat_area));

    LBP16_INIT_PACKET4(packet, CMD_READ_BOARD_INFO_ADDR16_INCR(sizeof(info_area)/2), 0);
    memset(&info_area, 0, sizeof(info_area));
    eth_socket_send_packet(&packet, sizeof(packet));
    eth_socket_recv_packet(&info_area, sizeof(info_area));

    if (info_area.LBP16_version >= 3) {
        LBP16_INIT_PACKET4(cmds[4], CMD_READ_AREA_INFO_ADDR16_INCR(LBP16_SPACE_TIMER, sizeof(mem_area)/2), 0);
        LBP16_INIT_PACKET4(packet, CMD_READ_TIMER_ADDR16_INCR(sizeof(timers_area)/2), 0);
        memset(&timers_area, 0, sizeof(timers_area));
        eth_socket_send_packet(&packet, sizeof(packet));
        eth_socket_recv_packet(&timers_area, sizeof(timers_area));
    }

    printf("LBP16 firmware info:\n");
    printf("  memory spaces:\n");
    for (i = 0; i < LBP16_MEM_SPACE_COUNT; i++) {
        u32 size;

        if ((cmds[i].cmd_lo == 0) && (cmds[i].cmd_hi == 0)) continue;
        memset(&mem_area, 0, sizeof(mem_area));
        eth_socket_send_packet(&cmds[i], sizeof(cmds[i]));
        eth_socket_recv_packet(&mem_area, sizeof (mem_area));

        printf("    %d: %.*s (%s, %s", i, sizeof(mem_area.name), mem_area.name, mem_types[(mem_area.size  >> 8) & 0x7F],
          mem_writeable[(mem_area.size & 0x8000) >> 15]);
        for (j = 0; j < 4; j++) {
            if ((mem_area.size & 0xFF) & 1 << j)
                printf(", %s)", acc_types[j]);
        }
        size = pow(2, mem_area.range & 0x3F);
        if (size < 1024) {
            printf(" [size=%u]", size);
        } else if ((size >= 1024) && (size < 1024*1024)) {
            printf(" [size=%uK]", size/1024);
        } else if (size >= 1024*1024) {
            printf(" [size=%uM]", size/(1024*1024));
        }
        if (((mem_area.size  >> 8) & 0x7F) >= 0xE)
            printf(", page size: 0x%X, erase size: 0x%X",
              (unsigned int) pow(2, (mem_area.range >> 6) & 0x1F), (unsigned int) pow(2, (mem_area.range >> 11) & 0x1F));
        printf("\n");
    }
 
    printf("  [space 0] Hostmot2\n");
    printf("  [space 2] Ethernet eeprom:\n");
    printf("    mac address: %02X:%02X:%02X:%02X:%02X:%02X\n", HI_BYTE(eth_area.mac_addr_hi), LO_BYTE(eth_area.mac_addr_hi),
      HI_BYTE(eth_area.mac_addr_mid), LO_BYTE(eth_area.mac_addr_mid), HI_BYTE(eth_area.mac_addr_lo), LO_BYTE(eth_area.mac_addr_lo));
    printf("    ip address: %d.%d.%d.%d\n", HI_BYTE(eth_area.ip_addr_hi), LO_BYTE(eth_area.ip_addr_hi), HI_BYTE(eth_area.ip_addr_lo), LO_BYTE(eth_area.ip_addr_lo));
    printf("    board name: %.*s\n", sizeof(eth_area.name), eth_area.name);
    printf("    user leds: %s\n", led_debug_types[eth_area.led_debug & 0x1]);

    printf("  [space 3] FPGA flash eeprom:\n");
    lbp16_read(CMD_READ_FLASH_IDROM, FLASH_ID_REG, &flash_id, 4);
    printf("    flash id: 0x%02X %s\n", flash_id, eeprom_get_flash_type(flash_id));

    if (info_area.LBP16_version >= 3) {
        printf("  [space 4] timers:\n");
        printf("     uSTimeStampReg: 0x%04X\n", timers_area.uSTimeStampReg);
        printf("     WaituSReg: 0x%04X\n", timers_area.WaituSReg);
        printf("     HM2Timeout: 0x%04X\n", timers_area.HM2Timeout);
    }

    printf("  [space 6] LBP16 status/control:\n");
    printf("    packets recived: all %d, UDP %d, bad %d\n", stat_area.RXPacketCount, stat_area.RXUDPCount, stat_area.RXBadCount);
    printf("    packets sended: all %d, UDP %d, bad %d\n", stat_area.TXPacketCount, stat_area.TXUDPCount, stat_area.TXBadCount);
    printf("    parse errors: %d, mem errors %d, write errors %d\n", stat_area.LBPParseErrors, stat_area.LBPMemErrors, stat_area.LBPWriteErrors);
    printf("    error flags: 0x%04X\n", stat_area.ErrorReg);
    printf("    debug LED ptr: 0x%04X\n", stat_area.DebugLEDPtr);
    printf("    scratch: 0x%04X\n", stat_area.Scratch);

    printf("  [space 7] LBP16 info:\n");
    printf("    board name: %.*s\n", sizeof(info_area.name), info_area.name);
    printf("    LBP16 version %d\n", info_area.LBP16_version);
    printf("    firmware version %d\n", info_area.firmware_version);
    printf("    IP address jumpers at boot: %s\n", boot_jumpers_types[info_area.jumpers]);
}
