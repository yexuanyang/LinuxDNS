#include "total.h"

/// <summary>
/// 名字解析器：传入DNS报文，得到解析之后的域名字符串
/// </summary>
/// <param name="packet">传入的DNS报文</param>
/// <param name="dest">保存解析后的域名字符串</param>
void Get_TLD(char *buf, char *dest,unsigned long long offset)
{
	char *bufT = buf + offset;
	int i = 0, j = 0,
	    k = 0; //i是报文里的下标，j是计数是几，k是目标位置下标，
	while (bufT[i] != 0 &&
	       (unsigned char)bufT[i] !=
		       0xc0) { //没到达域名结尾   (c开头的为域名压缩表示)
		if (bufT[i] > 0 &&
		    bufT[i] <=
			    63) //如果buf[i]为计数字节（例如3www5baidu3com中的3，5，3）
		{
			for (j = bufT[i], i++; j > 0; j--, i++, k++)
				dest[k] = bufT[i];
		}
		if (bufT[i] != 0) //如果没结束就在dest里加个'.'
		{
			dest[k] = '.';
			k++;
		}
	}
	dest[k] = '\0';
	if (bufT[i] != 0){//压缩形式表示的域名
		unsigned short offsetT = (((unsigned short)bufT[i]) << 8 |
					 (unsigned char)bufT[i + 1]) &
					0x3fff;
		char rest[DNameMaxLen];
		Get_TLD(buf,rest,offsetT);
		strcat_s(dest, strlen(dest) + strlen(rest) + 1, rest);
	}
}