#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "udp.h"
#include "cli.h"
#include "rdp.h"
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

bool SENDER_SEQ_NUM = 0;
bool break_flag = 0;

bool SEQ_NUM = 0;
bool ACK_NUM = 0;

void rdp_sender(struct udp_pcb *pcb, struct pbuf *p)
{
    printf("Sending... \n");
    uchar pcb_loc_ip[4];
    gHtonl(pcb_loc_ip, pcb->local_ip);
    printf("Sending pcb local ip: %u.%u.%u.%u\n", pcb_loc_ip[3], pcb_loc_ip[2], pcb_loc_ip[1], pcb_loc_ip[0]);
    printf("Sending pcb local port: %u\n", pcb->local_port);
    uchar pcb_rem_ip[4];
    printf("Sending pcb remote ip: %u.%u.%u.%u\n", pcb_rem_ip[3], pcb_rem_ip[2], pcb_rem_ip[1], pcb_rem_ip[0]);
    printf("Sending pcb remote port: %u\n", pcb->remote_port);

    int ack = send_check(pcb, p, SENDER_SEQ_NUM);

    while (1)
    {
        if (ack == -1)
        {
            printf("Time out! \n");
            ack = send_check(pcb, p, SENDER_SEQ_NUM);
            //resend the packet
        }
        else if (ack == 1)
        {
            SENDER_SEQ_NUM = !SENDER_SEQ_NUM;
            printf("Successful! \n");
            //send next packet
            break;
        }
    }
}

int send_check(struct udp_pcb *pcb, struct pbuf *p, int seqnum)
{
    struct timeval now, then;
    float time_use = 0;

    printf("local_port: %u\n", pcb->local_port);
    if (seqnum == 0)
    {
        pcb->local_port = pcb->local_port & 0x3fff;
        printf("0: %u", pcb->local_port);
    }

    if (seqnum == 1)
    {
        pcb->local_port = pcb->local_port & 0xbfff;
        printf("1: %u", pcb->local_port);
    }

    err_t err = udp_send(pcb, p);

    pcb->local_port = pcb->local_port | 0xc000;

    sleep(10);

    if (break_flag == 1)
    {
        break_flag = 0;
        return 1;
    }

    return -1;
}

void setBreakFlag(bool b)
{
    printf("Here %d \n", b);
    break_flag = b;
}

void checkPacket(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port)
{
    // check for target sequenceNum
    /**
  * Based on the UDP_LOCAL_PORT_RANGE_START and UDP_LOCAL_PORT_RANGE_END fields introduced in udp.c,
  * we assume the sequence number is the first bit of pcb local port
  * and ack number is the second bit of pcb local port
  * 
  */

    uint16_t port_with_flags = port;
    printf("Port with flag = %u\n", port);

    // bit and find seq flag
    uint16_t temp = port_with_flags & 0x8000;
    printf("temp = %u\n", temp);
    bool seq_num = temp > 0;
    printf("seq num waiting for = %d\n", SEQ_NUM);
    printf("seq num = %d\n", seq_num);

    /**
  // bit and find ack flag
  temp = port_with_flags & 0x4000;
  bool ack_num = temp > 0;
  printf("ack num waiting for = %d\n", ACK_NUM);
  printf("ack num = %d\n", ack_num);
*/

    // reset the correct port number
    pcb->local_port = (port_with_flags | 0xc000);
    printf("Correct port number %u\n", pcb->local_port);

    //connect
    uchar ipaddr_network_order[4];
    gHtonl(ipaddr_network_order, addr);
    udp_connect(arg, ipaddr_network_order, port);

    // if it is a packet
    // if (ack_num == 0)
    // {
    // check for seq num
    // not the correct one, send ack packet with seq = ! seq, ack = 1
    if (seq_num != SEQ_NUM)
    {
        printf("receive packet, NOT the seq number waiting for\n");
    }
    // otherwise, send the ack packet with seq = seq, ack = 1
    else
    {
        printf("receive packet, IS the seq number waiting for\n");

        // shift seq num
        SEQ_NUM = !SEQ_NUM;

        //udp_recv_callback(arg, pcb, p, addr, port);
        printf("%s", (char *)p->payload);

        break_flag = 1;
    }
}


void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port)
{
    // check validation
    checkPacket(arg, pcb, p, addr, port);
}
