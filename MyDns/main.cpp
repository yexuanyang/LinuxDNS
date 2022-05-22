#include "total.h"

//全局变量定义
int level = 0;                            //调试等级
ID_TRANS_CELL trans_table[ID_TRANS_SIZE]; /*The ID_TRANS_TABLE*/
int trans_count = 0;                      /*The cells of trans_table now*/
SOCKET inDNS, outDNS;
sockaddr_in local_name, extern_name;
sockaddr_in client, extern_dns;
int length_client = sizeof sockaddr_in;
char fileName[129] = "dest.txt";
char DNSServerIp[16] = "192.168.1.1";
DNIPList **local_dniplist = (DNIPList **)malloc(sizeof(DNIPList *)),
	 **extern_dniplist = (DNIPList **)malloc(sizeof(DNIPList *));

void getLevel(int argc, char *argv[])
{
	BOOL setDNS = false, setFile = false;
	if (argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == 'd')
			level = 1;
		else if (argv[1][1] == 'D')
			level = 2;
		if (argc > 2) {
			setDNS = true;
			strcpy_s(DNSServerIp, sizeof DNSServerIp, argv[2]);
		}
		if (argc > 3) {
			setFile = true;
			strcpy_s(fileName, sizeof fileName, argv[3]);
		}
	}
	if (setDNS) {
		printf("set DNS server: %s\n", DNSServerIp);
	}
	if (setFile) {
		printf("set file : %s\n", fileName);
	}
	printf("Debug Level:%d\n", level);
}

void showBuffer(char *buf, int length)
{
	printf("获取到的包数据长度：%d\n获取的包数据：\n ", length);
	for (int i = 0; i < length; i++) {
		printf("%02x ", (unsigned char)buf[i]);
		if ((i + 1) % 20 == 0) {
			printf("\n");
		}
	}
	printf("\n\n");
}

void addToExternDniplist(DNIPList **extern_dniplist, DNIPList *newNode)
{
	DNIPList *current = *extern_dniplist;
	while (current->nextPtr) {
		current = current->nextPtr;
	}
	current->nextPtr = newNode;
}

void receiveFromLocal()
{
	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	int dataLength = -1;
	dataLength = recvfrom(inDNS, buf, BUFSIZE, 0, (SOCKADDR *)&client,
			      &length_client);
	if (dataLength > -1) {
		DNS_PACKET packet = receiveDNS(buf);
		char url[DNameMaxLen];
		Get_TLD(buf+12, url);
		if (level > 0) {
			PrintTime();
			printf("客户端IP：  %s:%u\n",
			       inet_ntoa(client.sin_addr), client.sin_port);
			printf("询问的域名: %s\n", url);
			if (level > 1) {
				showBuffer(buf, dataLength);
				Show_DNSPacket(packet,buf);
			}
		}
		char *ip = Ip_str(local_dniplist, extern_dniplist, url);
		//在本地或者网络缓存表找到
		if (strcmp(ip ,"failed") != 0) {
			if (level > 0) {
				PrintTime();
				printf("url:%s -> IP: %s\n", url, ip);
			}
			char sendBuf[BUFSIZE];
			memcpy(sendBuf, buf, dataLength);
			unsigned short _16bitflag = htons(0x8180);
			unsigned short _16bitANcount;
			memcpy(&sendBuf[2], &_16bitflag,
			       sizeof(unsigned short));

			if (strcmp(ip, "0.0.0.0") == 0) {
				_16bitANcount = htons(0x0000);
			} else {
				_16bitANcount = htons(0x0001);
			}
			memcpy(&sendBuf[6], &_16bitANcount,
			       sizeof(unsigned short));

			int curlen = 0;
			char answer[16];
			unsigned short Name = htons(0xc00c);
			unsigned short TypeA = htons(0x0001);
			unsigned short ClassA = htons(0x0001);
			unsigned long TTL = htons(0x78); // 120s
			unsigned short Datalen = htons(0x0004);
			unsigned long IPAddress = inet_addr(ip);
			memcpy(answer + curlen, &Name, sizeof(unsigned short));
			curlen += sizeof(unsigned short);
			memcpy(answer + curlen, &TypeA, sizeof(unsigned short));
			curlen += sizeof(unsigned short);
			memcpy(answer + curlen, &ClassA,
			       sizeof(unsigned short));
			curlen += sizeof(unsigned short);
			memcpy(answer + curlen, &TTL, sizeof(unsigned long));
			curlen += sizeof(unsigned long);
			memcpy(answer + curlen, &Datalen,
			       sizeof(unsigned short));
			curlen += sizeof(unsigned short);
			memcpy(answer + curlen, &IPAddress,
			       sizeof(unsigned long));
			curlen += sizeof(unsigned long);

			memcpy(&sendBuf[12], answer, sizeof answer);

			dataLength = sendto(inDNS, sendBuf, curlen + dataLength,
					    0, (SOCKADDR *)&client,
					    length_client);

			if (dataLength < 0) {
				printf("发送包失败\n");
			}

			if (level > 0) {
				printf("发送回应包： url:%s -> ip:%s\n", url,
				       ip);
			}

		} else { //往外部DNS发
			printf("url: %s 在本地DNS服务器不能解析，将发送至外部DNS\n",
			       url);
			unsigned short pid;
			memcpy(&pid, buf, sizeof(unsigned short));
			unsigned short nid =
				generate_new_id(pid, client, 10, url);
			if (nid == -1 && level > 0) {
				printf("buffer full");
			} else {
				memcpy(buf, &nid, sizeof(nid));
				dataLength = sendto(outDNS, buf, dataLength, 0,
						    (SOCKADDR *)&extern_name,
						    sizeof extern_name);
				if (level > 0) {
					printf("向外部DNS发送请求.  url: %s\n",
					       url);
				}
			}
		}
	}
}

void receiveFromExtern()
{
	char buf[BUFSIZE];
	char url[DNameMaxLen];
	memset(buf, 0, BUFSIZE);
	int datalength = -1;
	datalength = recvfrom(outDNS, buf, BUFSIZE, 0, (SOCKADDR *)&extern_dns,
			      &length_client);

	if (datalength > -1) {
		if (level > 0) {
			printf("外部DNS服务器IP：%s\n",
			       inet_ntoa(extern_dns.sin_addr));

			PrintTime();
			if (level > 1) {
				showBuffer(buf, datalength);
			}
		}

		unsigned short pid;
		memcpy(&pid, buf, sizeof(unsigned short));
		int indexInTable = pid;
		memcpy(buf, &trans_table[indexInTable].last_ID,
		       sizeof(unsigned short));
		trans_count--;
		if (level > 1) {
			printf("转换表项数:%d \n", trans_count);
		}
		trans_table[indexInTable].done = true;
		DNIPList newNode;
		DNS_PACKET packet = receiveDNS(buf);
		Show_DNSPacket(packet,buf);
		memcpy(newNode.dn, packet.AN->name, sizeof(newNode.dn));
		newNode.expire_time = packet.AN->TTL;
		memcpy(newNode.ip, packet.AN->Rdata, sizeof(unsigned int));
		newNode.nextPtr = NULL;
		addToExternDniplist(extern_dniplist, &newNode);

		client = trans_table[indexInTable].client;
		datalength = sendto(inDNS, buf, datalength, 0,
				    (SOCKADDR *)&client, length_client);
	}
}

int main(int argc, char *argv[])
{	
	char buf[BUFSIZE];
	char ip[16];

	print_team_msg();
	getLevel(argc, argv);

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
		return 0;

	inDNS = socket(AF_INET, SOCK_DGRAM, 0);
	outDNS = socket(AF_INET, SOCK_DGRAM, 0);
	if (inDNS < 0)
		printf("create socket failed! error code:%d",WSAGetLastError());
	if(outDNS < 0)
		printf("create socket failed! error code:%d",
		       WSAGetLastError());
	

	Read_scheurl(local_dniplist, extern_dniplist);

	memset(&local_name, 0, sizeof local_name);
	memset(&extern_name, 0, sizeof extern_name);

	local_name.sin_family = AF_INET;
	local_name.sin_addr.s_addr = INADDR_ANY;
	local_name.sin_port = htons(DNS_PORT);

	extern_name.sin_family = AF_INET;
	extern_name.sin_addr.s_addr = inet_addr(DNSServerIp);
	extern_name.sin_port = htons(DNS_PORT);

	for (int i = 0; i < ID_TRANS_SIZE; i++) {
		trans_table[i].last_ID = 0;
		trans_table[i].done = TRUE;
		trans_table[i].expire_time = 0;
		memset(&(trans_table[i].client), 0, sizeof(SOCKADDR_IN));
	}

	int unblock = 1;
	if(ioctlsocket(inDNS, FIONBIO, (u_long FAR *)&unblock))
		printf("ioctlsocket failed! error code:%d\n",WSAGetLastError()); //本地套接字非阻塞
	
	if(ioctlsocket(outDNS, FIONBIO, (u_long FAR *)&unblock))
		printf("ioctlsocket failed! error code:%d\n",
		       WSAGetLastError()); //外部套接字非阻塞
	int reuse = 1;
	if(setsockopt(
		inDNS, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
		sizeof(reuse)))
		printf("setsockopt failed! error code:%d\n",WSAGetLastError()); //SO_REUSEADDR允许在同一端口上启动同一服务器的多个实例
	//SOL_SOCKET在套接字级别上设置选项
	

	if (bind(inDNS, (SOCKADDR *)&local_name, sizeof(SOCKADDR)) &&
	    bind(outDNS, (SOCKADDR *)&extern_name, sizeof(SOCKADDR)) ) {
		printf("ERROR! BIND FAILED! error code:%d\n",WSAGetLastError());
		exit(1);
	}else {
		printf("BIND SUCCESSFULLY!\n");
	}

	while (1) {
		receiveFromLocal();
		receiveFromExtern();
	}

	closesocket(inDNS);
	closesocket(outDNS);
	WSACleanup();
	return 0;
}