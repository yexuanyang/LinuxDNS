#include "total.h"

/// <summary>
/// 名字解析器：传入DNS报文，得到解析之后的域名字符串
/// </summary>
/// <param name="packet">传入的DNS报文</param>
/// <param name="dest">保存解析后的域名字符串</param>
void Get_TLD(char *buf, char *dest)
{
	int i = 0, j = 0,
	    k = 0; //i是报文里的下标，j是计数是几，k是目标位置下标，
	while (buf[i] != 0) {
		if (buf[i] > 0 &&
		    buf[i] <=
			    63) //如果buf[i]为计数字节（例如3www5baidu3com中的3，5，3）
		{
			for (j = buf[i], i++; j > 0; j--, i++, k++)
				dest[k] = buf[i];
		}
		if (buf[i] != 0) //如果没结束就在dest里加个'.'
		{
			dest[k] = '.';
			k++;
		}
	}
	dest[k] = '\0';
}