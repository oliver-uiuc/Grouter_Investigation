void rdp_sender(struct udp_pcb *pcb, struct pbuf *p);
int send_check(struct udp_pcb *pcb, struct pbuf *p, int seqnum);
void setBreakFlag(bool b);
void checkPacket(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port);
void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port);