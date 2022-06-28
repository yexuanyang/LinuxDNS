#include "total.h"


void Get_TLD(char *buf, char *dest,unsigned long long offset)
{
	char *bufT = buf + offset;
	int i = 0, j = 0,
	    k = 0; //i is the index in message; j is the count ; k is the target location index
	while (bufT[i] != 0 &&
	       (unsigned char)bufT[i] !=
		       0xc0) { //The end of the domain name is not reached (the domain name beginning with C is compressed)
		if (bufT[i] > 0 &&
		    bufT[i] <=
			    63) //buf[i] is count bytes (for example, 3, 5, 3 in 3www5baidu3com) 
		{
			for (j = bufT[i], i++; j > 0; j--, i++, k++)
				dest[k] = bufT[i];
		}
		if (bufT[i] != 0) //If it's not over, add a '.' to dest
		{
			dest[k] = '.';
			k++;
		}
	}//exit when reach end or meet compressed domain name
	dest[k] = '\0';
	if (bufT[i] != 0){// Compressed domain name 
		unsigned short offsetT = (((unsigned short)bufT[i]) << 8 |
					 (unsigned char)bufT[i + 1]) &
					0x3fff;// get the offset
		char rest[DNameMaxLen];
		Get_TLD(buf,rest,offsetT);
		strcat(dest,rest);
	}
}