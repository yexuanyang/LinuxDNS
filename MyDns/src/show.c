
#include "total.h"

extern int level;

void PrintTime()
{
	char curTime[25];
	time_t t;
	time(&t);
	struct tm defaultTm = {0, 0, 0, 1, 1, 0, 0, 0, 0};
	struct tm *time = &defaultTm;
	#if _WIN64
	localtime_s(time, &t);
	#elif __linux__
	time = localtime(&t);
	#endif
	strftime(curTime, sizeof(curTime), "%c",
		 time);
	printf("时间：%s\n", curTime);
	return;
}

void PrintAnswer(DNS_PACKET packet,char *buf)
{
	
	for (unsigned j = 0; j < packet.header.ancount; j++) {
		unsigned short offset = ( ((unsigned short)packet.AN[j].name[0]) << 8 | (unsigned char)packet.AN[j].name[1] ) & 0x3fff;
		printf("<==========回答区段==========>\n\n");
		char url[DNameMaxLen];
		Get_TLD(buf, url,offset);
		printf("Name: %s \t", url);
		if (packet.AN[j].RRtype == (unsigned)1)
			printf("IP: %u.%u.%u.%u\t", packet.AN[j].Rdata[0] & 0x000000ff,
			       packet.AN[j].Rdata[1] & 0x000000ff,
			       packet.AN[j].Rdata[2] & 0x000000ff,
			       packet.AN[j].Rdata[3] & 0x000000ff);
		PrintTime();
		if (level == 2) {
			printf("Type: %u \tClass: %u\t TTL: %u\t DataLen: %u\n",
						packet.AN[j].RRtype,
						packet.AN[j].RRclass, packet.AN[j].TTL,
						packet.AN[j].dataLen);
			if (packet.AN[j].RRtype == (unsigned)5) {
				char *cname = (char *)malloc(
					sizeof(char) * DNameMaxLen);
				unsigned long long offsetRdata = packet.AN[j].Rdata - buf;
				Get_TLD(buf, cname , offsetRdata);
				printf("CNAME: \t%s\t", cname);
				PrintTime();
				free(cname);
			}
		}
	}
	
	if (level == 2) {
		
		
		for (unsigned i = 0; i < packet.header.nscount; i++) {
			printf("<==========授权区段==========>\n\n");
			printf("Name: %s \t", packet.NS[i].name);
			PrintTime();
			printf("Type: %u \tClass: %u\t TTL: %u\t DataLen: %u\n",
			       packet.NS[i].RRtype, packet.NS[i].RRclass,
			       packet.NS[i].TTL, packet.NS[i].dataLen);
		}
		
		
		for (unsigned i = 0; i < packet.header.arcount; i++) {
			printf("<==========额外资源区段==========>\n\n");
			printf("Name: %s \t", packet.AR[i].name);
			PrintTime();
			printf("Type: %u \tClass: %u\t TTL: %u\t DataLen: %u\n",
			       packet.AR[i].RRtype, packet.AR[i].RRclass,
			       packet.AR[i].TTL, packet.AR[i].dataLen);
		}
	}
		
}

void Show_DNSPacket(DNS_PACKET packet,char *buf)
{
	if (level == 2) {
		printf("\n ID: %u \n QR: %x \n OPCODE: %x \n AA: %x \n TC: %x \n RD: %x \n RA: %x \n zero: %x \n rcode: %x \n\
 问题数: %u \n 资源记录数: %u \n 授权资源记录数: %u \n 额外资源记录数: %u \n",
		       packet.header.id, packet.header.qr, packet.header.opcode,
		       packet.header.aa, packet.header.tc, packet.header.rd,
		       packet.header.ra, packet.header.z, packet.header.rcode,
		       packet.header.qdcount, packet.header.ancount,
		       packet.header.nscount, packet.header.arcount);
	} else if (level == 1) {
		printf("\nID: %u \n ", packet.header.id);
	}
	PrintAnswer(packet,buf);
}