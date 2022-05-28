#include "total.h"

extern int level;
extern int trans_count;
extern ID_TRANS_CELL trans_table[ID_TRANS_SIZE];
unsigned short generate_new_id(unsigned short id, sockaddr_in cli, int ttl,
			       char *url)
{
	/*
	* 注意expire_time是指的过期的时间 而不是距离过期的时间
	*/
	time_t nowtime=time(NULL);//获取当前时间	
	int i = 0;
	for (i = 0; i < trans_count; i++) {
		if (trans_table[i].done ||
		    trans_table[i].expire_time < nowtime) { //如果解析完毕或者生存时间结束则直接进行覆盖
			if (trans_table[i].expire_time != -1 &&trans_table[i].expire_time < nowtime && level>=1) {//调试等级大于1则输出
				printf("The request packet with URL %s expired and the record has been discarded\n",
				       trans_table[i].url);
			}
			//进行覆盖
			trans_table[i].last_ID = id;
			trans_table[i].client = cli;
			trans_table[i].done = false;
			trans_table[i].expire_time = nowtime + ttl;
			strcpy_s(trans_table[i].url, sizeof(trans_table[i].url), url);
			return i;//返回当前所在的index作为id
		}
	}
	if (trans_count < ID_TRANS_SIZE) { //如果表中的数量小于32则直接添加进
		trans_table[trans_count].last_ID = id;
		trans_table[trans_count].client = cli;
		trans_table[trans_count].done = false;
		trans_table[trans_count].expire_time = nowtime + ttl;
		strcpy_s(trans_table[i].url, sizeof(trans_table[i].url), url);
		trans_count++;
		return trans_count - 1; //返回在ID转换表里面的顺序
	} else
		return -1;
}