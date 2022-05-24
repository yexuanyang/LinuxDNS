//引用头文件和导入库
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <time.h>
#include <fstream>
#include <stdbool.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

//常量定义
#define ID_TRANS_SIZE 32 /* The max size of ID_TRANS_TABLE*/
#define DNS_PORT 53
#define DNameMaxLen 129

//宏定义
#define BUFSIZE 1024

//结构体定义
typedef struct {
	unsigned id : 16;      /* query identification number */
	unsigned qr : 1;       /* response flag */
	unsigned opcode : 4;   /* purpose of message */
	unsigned aa : 1;       /* authoritive answer */
	unsigned tc : 1;       /* truncated message */
	unsigned rd : 1;       /* recursion desired */
	unsigned rcode : 4;    /* response code */
	unsigned z : 3;        /* unused bits, must be ZERO */
	unsigned ra : 1;       /* recursion available */
	unsigned qdcount : 16; /* number of question entries */
	unsigned ancount : 16; /* number of answer entries */
	unsigned nscount : 16; /* number of authority entries */
	unsigned arcount : 16; /* number of resource entries */
} HEADER;

//问题字段
typedef struct {
	char qname[DNameMaxLen]; //查询名称
	unsigned qtype : 16;  //查询类型
	unsigned qclass : 16; //查询类
} QUESTION;
//
typedef struct {
	char name[DNameMaxLen]; //域名
	unsigned RRtype : 16;  //类型
	unsigned RRclass : 16; //类
	unsigned TTL : 32;     //生存时间
	unsigned dataLen : 16; //数据长度
	char *Rdata;
} RR;

typedef struct {
	HEADER header;       //DNS报头
	QUESTION *questions; //问题
	RR *AN;              //回答
	RR *NS;              //授权名字服务器
	RR *AR;              //额外资源
} DNS_PACKET;

typedef struct DNIPList { //这里进行更改
	char ip[16]; //IP地址 进行了修改 ip地址的格式 xxx.xxx.xxx.xxx 16个字节应该可以使用
	char dn[DNameMaxLen]; //域名
	int expire_time;   //过期时间
	DNIPList *nextPtr; //指向下一结点的指针
	int length;
} DNIPList;                //标准：链表需要有不含值的头节点

typedef struct {
	unsigned short last_ID; /* The old id*/
	BOOL done;          /*Mark whether the request was analysed completely*/
	SOCKADDR_IN client; /*Requestor socket address*/
	char url[DNameMaxLen]; /*URL*/
	int expire_time;    /*The time to die*/
} ID_TRANS_CELL;

//函数声明

/// <summary>
/// 查找域名对应的IP地址，返回字符串形式IP地址，并输出提示信息
/// 查找失败：返回failed字符串
/// 查出黑名单：输出“该域名在黑名单中”
/// </summary>
/// <param name="Local_DNIPList">	本地IP-域名对照表的链表头		</param>
/// <param name="Temp_DNIPList">	网络缓存生成的临时IP-域名对照表链表头	</param>
/// <param name="url">	要查询的域名	</param>
/// <returns>	字符串形式IP地址		</returns>
char *Ip_str(DNIPList **Local_DNIPList, DNIPList **Temp_DNIPList, char *url);

/// <summary>
/// 按照level输出DNS报文信息
/// </summary>
/// <param name="packet">要显示的DNS报文</param>
/// <param name="level">调试等级</param>
void Show_DNSPacket(DNS_PACKET packet, char *buf);

///读取本地的dns对照表，并把空的网络对照表链接到本地的dns表之后
void Read_scheurl(DNIPList **local_dniplist, DNIPList **extern_dniplist);

/// <summary>
/// 名字解析器,传入DNS报文,得到解析之后的域名字符串
/// </summary>
/// <param name="packet">传入的DNS报文</param>
/// <param name="dest">保存解析后的域名字符串</param>
void Get_TLD(char *buf, char *dest,unsigned long long offset);

/// <summary>
/// 获取DNS报文：返回DNS报文结构体
/// </summary>
/// <param name="buf">保存从客户端获取的字符数组</param>
DNS_PACKET receiveDNS(char *buf);

/// <summary>
/// 获取域名的字节长度
/// </summary>
int Get_TLDLength(char *p);

unsigned short generate_new_id(unsigned short id, sockaddr_in cli, int ttl,
			       char *url);

void print_team_msg();

void getLevel(int argc, char *argv[]);

void PrintTime();