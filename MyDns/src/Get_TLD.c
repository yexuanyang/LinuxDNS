#include "total.h"


void Get_TLD(char *buf, char *dest,unsigned long long offset)
{
	char *bufT = buf + offset;
	int i = 0, j = 0,
	    k = 0; //i�Ǳ�������±꣬j�Ǽ����Ǽ���k��Ŀ��λ���±꣬
	while (bufT[i] != 0 &&
	       (unsigned char)bufT[i] !=
		       0xc0) { //û����������β   (c��ͷ��Ϊ����ѹ����ʾ)
		if (bufT[i] > 0 &&
		    bufT[i] <=
			    63) //���buf[i]Ϊ�����ֽڣ�����3www5baidu3com�е�3��5��3��
		{
			for (j = bufT[i], i++; j > 0; j--, i++, k++)
				dest[k] = bufT[i];
		}
		if (bufT[i] != 0) //���û��������dest��Ӹ�'.'
		{
			dest[k] = '.';
			k++;
		}
	}
	dest[k] = '\0';
	if (bufT[i] != 0){//ѹ����ʽ��ʾ������
		unsigned short offsetT = (((unsigned short)bufT[i]) << 8 |
					 (unsigned char)bufT[i + 1]) &
					0x3fff;
		char rest[DNameMaxLen];
		Get_TLD(buf,rest,offsetT);
		strcat(dest,rest);
	}
}