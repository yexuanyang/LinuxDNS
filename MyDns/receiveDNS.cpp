#include "total.h"


extern struct sockaddr_in client;
extern SOCKET inDNS;
/// <summary>
/// 获取DNS报文：不传参数，从客户端接受，返回DNS报文结构体
/// </summary>
/// <param name="buf">从客户端获取的字符数组的头指针的地址</param>
DNS_PACKET receiveDNS(char *buf)
{
	DNS_PACKET packet;
	packet.AN = NULL;
	packet.AR = NULL;
	packet.NS = NULL;
	packet.questions = NULL;
	//char buf[BUFSIZE],BUFSIZE值为1024，为最大报文缓存大小，buf暂存报文
	int DNLength;                 //域名的字节长度
	int lenOfClient = sizeof client;
	char *p = buf;

	memcpy(&packet.header, buf, 12); //buf中前12个字符直接填入报文头
	unsigned short id = ntohs(packet.header.id);
	memcpy(&packet.header, &id, sizeof(id));
	unsigned short qdcount = ntohs(packet.header.qdcount);
	memcpy((char*) & packet.header + 4, &qdcount, sizeof(qdcount));
	unsigned short ancount = ntohs(packet.header.ancount);
	memcpy((char*) & packet.header + 6, &ancount, sizeof(ancount));
	unsigned short nscount = ntohs(packet.header.nscount);
	memcpy((char*) & packet.header + 8, &nscount, sizeof(nscount));
	unsigned short arcount = ntohs(packet.header.arcount);
	memcpy((char*) & packet.header + 10, &arcount, sizeof(arcount));

	if (qdcount != 0) {
		packet.questions = (QUESTION *)malloc(sizeof(QUESTION) *
								qdcount);
		if (packet.questions != NULL) {
			p = buf + 12; //指针p指向questions部分
			for (unsigned i = 0; i < qdcount;
					i++) { //将数据填入question部分
				DNLength = Get_TLDLength(p); //获取域名的字节长度
				memcpy(packet.questions[i].qname, p, DNLength);//name
				unsigned short type;
				memcpy(&type,p + DNLength, 2);//type
				packet.questions[i].qtype = ntohs(type);

				unsigned short Class;
				memcpy(&Class,
				       p + DNLength + 2, 2);//class
				packet.questions[i].qclass = ntohs(Class);
				
				p = p + DNLength + 4;
			}
		}
	}
	
	if (ancount != 0) {
		packet.AN = (RR *)malloc(sizeof(RR) * ancount);
		if (packet.AN != NULL) {
			for (unsigned i = 0; i < ancount; i++) { //将数据填入AN部分
				DNLength = 2;
				memcpy(packet.AN[i].name, p, DNLength);//name

				
				unsigned short RRtype;
				memcpy(&RRtype,p + DNLength, 2); //RRtype	
				packet.AN[i].RRtype = ntohs(RRtype);

				unsigned short RRclass;
				memcpy(&RRclass,
				       p + DNLength + 2,
					   2);		//RRclass
				packet.AN[i].RRclass = ntohs(RRclass);	
			

				unsigned TTL ;
				memcpy(&TTL,
				       p + DNLength + 4, 4); //TTL
				
				packet.AN[i].TTL = ntohl(TTL);

				unsigned short datalen;
				memcpy(&datalen,
				       p + DNLength + 8,
					   2); //dataLen
				
				packet.AN[i].dataLen = ntohs(datalen);

				packet.AN[i].Rdata = (char *)malloc(
					sizeof(char) * packet.AN[i].dataLen);
				memcpy(packet.AN[i].Rdata, p + DNLength + 10,
				       packet.AN[i].dataLen);
				p = p + DNLength + 10 + packet.AN[i].dataLen;
			}
		}
	}
	
	if (nscount != 0) {
		packet.NS = (RR *)malloc(sizeof(RR) * nscount);
		if (packet.NS != NULL) {
			for (unsigned i = 0; i < nscount; i++) { //将数据填入NS部分
				DNLength = 2;
				memcpy(packet.NS[i].name, p, DNLength);//name

				unsigned short RRtype;
				memcpy(&RRtype, p + DNLength, 2); //RRtype
				packet.NS[i].RRtype = ntohs(RRtype);

				unsigned short RRclass;
				memcpy(&RRclass, p + DNLength + 2,
				       2); //RRclass
				packet.NS[i].RRclass = ntohs(RRclass);

				unsigned TTL;
				memcpy(&TTL, p + DNLength + 4, 4); //TTL

				packet.NS[i].TTL = ntohl(TTL);

				unsigned short datalen;
				memcpy(&datalen, p + DNLength + 8,
				       2); //dataLen

				packet.NS[i].dataLen = ntohs(datalen);

				packet.NS[i].Rdata = (char *)malloc(
					sizeof(char) * packet.NS[i].dataLen);
				memcpy(packet.NS[i].Rdata, p + DNLength + 10,
				       packet.NS[i].dataLen);
				p = p + DNLength + 10 + packet.NS[i].dataLen;
			}
		}
		
	}
	
	if (arcount != 0) {
		packet.AR = (RR *)malloc(sizeof(RR) * arcount);
		if (packet.AR != NULL) {
			for (unsigned i = 0; i < arcount; i++) { //将数据填入AR部分
				
				DNLength = 2;
				memcpy(packet.AR[i].name, p, DNLength); //name
				
				unsigned short RRtype;
				memcpy(&RRtype, p + DNLength, 2); //RRtype
				packet.AR[i].RRtype = ntohs(RRtype);

				unsigned short RRclass;
				memcpy(&RRclass, p + DNLength + 2,
				       2); //RRclass
				packet.AR[i].RRclass = ntohs(RRclass);

				unsigned TTL;
				memcpy(&TTL, p + DNLength + 4, 4); //TTL

				packet.AR[i].TTL = ntohl(TTL);

				unsigned short datalen;
				memcpy(&datalen, p + DNLength + 8,
				       2); //dataLen

				packet.AR[i].dataLen = ntohs(datalen);

				packet.AR[i].Rdata = (char *)malloc(
					sizeof(char) * packet.AR[i].dataLen);
				memcpy(packet.AR[i].Rdata, p + DNLength + 10,
				       packet.AR[i].dataLen);
				p = p + DNLength + 10 + packet.AR[i].dataLen;
			}
		}
		
	}
	
	return packet;
}

/// <summary>
/// 获得域名的字节长度
/// </summary>
/// <param name="p">存储域名的内存地址</param>
/// <returns>域名长度</returns>
int Get_TLDLength(char *p)
{
	int sum = 1, i = 0;
	while (p[i] != 0) {
		i++;
		sum++;
	}
	return sum;
}