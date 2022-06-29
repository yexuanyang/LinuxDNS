#include "total.h"


int level = 0;                            //debug level
ID_TRANS_CELL trans_table[ID_TRANS_SIZE]; /*The ID_TRANS_TABLE*/
int trans_count = 0;                      /*The cells of trans_table now*/
#if _WIN64
SOCKET inDNS, outDNS;
int length_client = sizeof (struct sockaddr_in);
#elif __linux__
int inDNS,outDNS;
socklen_t length_client = sizeof(struct sockaddr);
#endif
struct sockaddr_in local_name, extern_name;
struct sockaddr_in client, extern_dns;
char fileName[129] = "dest.txt";
char DNSServerIp[17] = "192.168.1.1";
DNIPList **local_dniplist = NULL,
	**extern_dniplist = NULL;
void getLevel(int argc, char *argv[])
{
	bool setDNS = false, setFile = false;
	if (argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == 'd')
			level = 1;
		else if (argv[1][1] == 'D')
			level = 2;
		if (argc > 2) {
			setDNS = true;
			memcpy(DNSServerIp, argv[2], sizeof DNSServerIp);
		}
		if (argc > 3) {
			setFile = true;
			memcpy(fileName, argv[3], sizeof fileName);
		}
	}
	if (setDNS) {
		printf(" set DNS server: %s\n", DNSServerIp);
	}
	if (setFile) {
		printf(" set file : %s\n", fileName);
	}
	printf(" Debug Level:%d\n", level);
}

void showBuffer(char *buf, int length)
{
	printf(" 获取到的包数据长度：%d\n获取的包数据:\n ", length);
	for (int i = 0; i < length; i++) {
		printf("%02x ", (unsigned char)buf[i]);
		if ((i + 1) % 40 == 0) {
			printf("\n");
		}
	}
	printf("\n\n");
}

void addToExternDniplist(DNIPList **extern_dniplist, DNIPList *newNode)
{
	DNIPList *currentHead = ( * extern_dniplist)->nextPtr;
	(*extern_dniplist)->nextPtr = newNode;
	newNode->nextPtr = currentHead;
	(*extern_dniplist)->length++;
}

void receiveFromLocal()
{
	char buf[BUFSIZE],url[DNameMaxLen];
	memset(buf, 0, BUFSIZE);
	int dataLength = -1;
	dataLength = recvfrom(inDNS, buf, BUFSIZE, 0, (struct sockaddr *)&client,
				&length_client);
	if (dataLength > -1) {
		printf("\n receive successfully!\n\n");
		DNS_PACKET packet = receiveDNS(buf);
		
		Get_TLD(buf, url,12);
		if (level > 0) {
			PrintTime();
			
			printf(" 客户端IP:  %s:%u\n",
				inet_ntoa(client.sin_addr), client.sin_port);
				
			printf(" 询问的域名: %s\n", url);
			if (level > 1) {
				showBuffer(buf, dataLength);
				Show_DNSPacket(packet,buf);
			}
		}
		char *ip = Ip_str(local_dniplist, extern_dniplist, url);// find in the local data or temporary data
		
		if (strcmp(ip ,"failed") != 0) {// successfully
			if (level > 0) {
				PrintTime();
				printf("url:%s -> IP: %s\n", url, ip);
			}
			char sendBuf[BUFSIZE];// answer message
			memcpy(sendBuf, buf, dataLength);// copy the head
			unsigned short _16bitflag = htons(0x8180);
			unsigned short _16bitANcount;
			memcpy(sendBuf+2, &_16bitflag,
				sizeof(unsigned short));//change the flag

			if (strcmp(ip, "0.0.0.0") == 0) {
				_16bitANcount = htons(0x0000);
			} else {
				_16bitANcount = htons(0x0001);
			}
			memcpy(sendBuf+ 6, &_16bitANcount,
				sizeof(unsigned short));//change the ANcount

			int curlen = 0;
			char answer[16];// resource record
			unsigned short Name = htons(0xc00c);// compressed 
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
						0, (struct sockaddr *)&client,
						length_client);

			if (dataLength < 0) {
				printf(" 发送包失败\n");
			}

			if (level > 0) {
				printf(" 发送回应包： url:%s -> ip:%s\n", url,
					ip);
			}

		} else { // send to outer DNS
			printf(" url: %s 在本地DNS服务器不能解析,将发送至外部DNS\n",
				url);
			unsigned short pid;
			memcpy(&pid, buf, sizeof(unsigned short));
			pid = ntohs(pid);

			unsigned short nid =
				generate_new_id(pid, client, DefaultExpireTime, url);
			nid = htons(nid);
			if (nid == (unsigned short)-1 && level > 0) {
				printf(" buffer full\t nid:%x\n",nid);
				exit(1);
			} else {
				memcpy(buf, &nid, sizeof(nid));
				dataLength = sendto(outDNS, buf, dataLength, 0,
							(struct sockaddr *)&extern_name,
							sizeof extern_name);
				if (level > 0) {
					printf(" 向外部DNS发送请求.  url: %s\n",
						url);
				}
				
			}
			
			
		}
		free(packet.AN);
		free(packet.AR);
		free(packet.NS);
		free(packet.questions);
	}
}

void receiveFromExtern()
{
	char buf[BUFSIZE];
	char url[DNameMaxLen];
	memset(buf, 0, BUFSIZE);
	int datalength = -1;
	datalength = recvfrom(outDNS, buf, BUFSIZE, 0, (struct sockaddr *)&extern_dns,
				&length_client);

	if (datalength > -1) {
		printf("\n receive form extern server successfully!\n\n");
		if (level > 0) {
			printf(" 外部DNS服务器IP:%s\n",
				inet_ntoa(extern_dns.sin_addr));

			PrintTime();
			if (level > 1) {
				showBuffer(buf, datalength);
			}
		}

		unsigned short pid;
		memcpy(&pid, buf, sizeof(unsigned short));
		pid = ntohs(pid);

		int indexInTable = pid;
		unsigned short uid = ntohs(trans_table[indexInTable].last_ID);
		
		memcpy(buf, &uid, sizeof(unsigned short));
		if(trans_count > 0)
			trans_count--;
		if (level > 1) {
			printf(" 转换表项数:%d \n", trans_count);
		}
		trans_table[indexInTable].done = true;
		DNIPList *newNode = (DNIPList*)malloc(sizeof(DNIPList));
		DNS_PACKET packet = receiveDNS(buf);
		Show_DNSPacket(packet,buf);
		if (packet.AN && packet.AN->RRtype == 1) {
			char name[DNameMaxLen];
			unsigned short offset =
				(((unsigned short)packet.AN->name[0]) << 8 |
				packet.AN->name[1]) &
				0x3fff;
			Get_TLD(buf , name,offset);
			memcpy(newNode->dn, name, sizeof(newNode->dn));
			time_t t;
			time(&t);
			newNode->expire_time = packet.AN->TTL + t;
			struct in_addr ip_addr;
			#if _WIN64
			ip_addr.S_un.S_addr = (unsigned)packet.AN->Rdata[0] << 24 |
					(unsigned)packet.AN->Rdata[1] << 16 & 0x00ff0000|
					(unsigned)packet.AN->Rdata[2] << 8 & 0x0000ff00|
					(unsigned)packet.AN->Rdata[3] & 0x000000ff;// get the IP address 
			#elif __linux__
			ip_addr.s_addr = (unsigned)packet.AN->Rdata[0] << 24 |
					(unsigned)packet.AN->Rdata[1] << 16 & 0x00ff0000|
					(unsigned)packet.AN->Rdata[2] << 8 & 0x0000ff00|
					(unsigned)packet.AN->Rdata[3] & 0x000000ff;
			#endif
			memcpy(newNode->ip, inet_ntoa(ip_addr),sizeof(newNode->ip));
			newNode->nextPtr = NULL;
			addToExternDniplist(extern_dniplist, newNode);
		}
		client = trans_table[indexInTable].client;
		datalength = sendto(inDNS, buf, datalength, 0,
					(struct sockaddr *)&client, length_client);
		free(packet.AN);
		free(packet.AR);
		free(packet.NS);
		free(packet.questions);
	}
}

int main(int argc, char *argv[])
{	
	char buf[BUFSIZE];
	char ip[16];
	local_dniplist = (DNIPList**)malloc(sizeof(DNIPList*));
	extern_dniplist = (DNIPList**)malloc(sizeof(DNIPList*));
	print_team_msg();
	getLevel(argc, argv);//get debug level

	//start Asynchronous winsocket
	#if _WIN64
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
		return 0;
	#endif

	inDNS = socket(AF_INET, SOCK_DGRAM, 0);
	outDNS = socket(AF_INET, SOCK_DGRAM, 0);
	if (inDNS < 0){
		#if _WIN64
		printf("create inDNS socket failed! error code:%d",WSAGetLastError());
		#elif __linux__
		printf("create inDNS socket failed! error information:%s\n",strerror(errno));
		#endif
	}
		
	if(outDNS < 0){
		#if _WIN64
		printf("create outDNS socket failed! error code:%d",
			WSAGetLastError());
		#elif __linux__
		printf("create outDNS socket failed! error information:%s\n",
				strerror(errno));
		#endif
	}
		
	

	Read_scheurl(local_dniplist, extern_dniplist);// load local data

	memset(&local_name, 0, sizeof local_name);
	memset(&extern_name, 0, sizeof extern_name);// extern_name is outer dns

	local_name.sin_family = AF_INET;
	local_name.sin_addr.s_addr = inet_addr("127.0.0.1");
	local_name.sin_port = htons(DNS_PORT);

	extern_name.sin_family = AF_INET;
	extern_name.sin_addr.s_addr = inet_addr(DNSServerIp);
	extern_name.sin_port = htons(DNS_PORT);

	for (int i = 0; i < ID_TRANS_SIZE; i++) {
		trans_table[i].last_ID = 0;
		trans_table[i].done = true;
		trans_table[i].expire_time = 0;
		memset(&(trans_table[i].client), 0, sizeof(struct sockaddr_in));
	}

	int unblock = 1;
	#if _WIN64
	if(ioctlsocket(inDNS, FIONBIO, (u_long FAR *)&unblock)||ioctlsocket(outDNS, FIONBIO, (u_long FAR *)&unblock))
		printf("ioctlsocket failed! error code:%d\n",WSAGetLastError()); //Local socket non blocking
	
	#elif __linux__
	if(fcntl(inDNS,F_SETFL,O_NONBLOCK)||fcntl(outDNS,F_SETFL,O_NONBLOCK)){
		printf("fcntl failed! error information:%s\n",strerror(errno)); //Local socket non blocking
	}
	#endif
	int reuse = 1;
	if(setsockopt(
		inDNS, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
		sizeof(reuse))){
			#if _WIN64
			printf("setsockopt failed! error code:%d\n",WSAGetLastError()); 
			//SO_REUSEADDR allows you to start multiple instances of the same server on the same port
			#elif __linux__
			printf("setsockopt failed! error information:%s\n",strerror(errno)); 
			//SO_REUSEADDR allows you to start multiple instances of the same server on the same port	
			#endif	
		}
		
	//SOL_SOCKET sets options at the socket level
	
	int in = bind(inDNS, (struct sockaddr *)&local_name, sizeof(struct sockaddr));
	if ( in) {
		#if _WIN64
		if (in)
			printf("ERROR! INDNS BIND FAILED! error code:%d\n",
				WSAGetLastError());
		#elif __linux__
		if(in)
			printf("ERROR! INDNS BIND FAILED! error information:%s\n",strerror(errno) );
		#endif
		exit(1);
	}else {
		printf("BIND SUCCESSFULLY!\n");
	}

	while (1) {
		receiveFromLocal();
		receiveFromExtern();
		DNIPList *prinTemp = NULL;
	}

	#if _WIN64
	closesocket(inDNS);
	closesocket(outDNS);
	WSACleanup();
	#elif __linux
	close(inDNS);
	close(outDNS);
	#endif

	return 0;
}