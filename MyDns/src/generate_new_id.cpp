#include "total.h"

extern int level;
extern int trans_count;
extern ID_TRANS_CELL trans_table[ID_TRANS_SIZE];
unsigned short generate_new_id(unsigned short id, sockaddr_in cli, int ttl,
			       char *url)
{
	/*
	* ע��expire_time��ָ�Ĺ��ڵ�ʱ�� �����Ǿ�����ڵ�ʱ��
	*/
	time_t nowtime=time(NULL);//��ȡ��ǰʱ��	
	int i = 0;
	for (i = 0; i < trans_count; i++) {
		if (trans_table[i].done ||
		    trans_table[i].expire_time < nowtime) { //���������ϻ�������ʱ�������ֱ�ӽ��и���
			if (trans_table[i].expire_time != -1 &&trans_table[i].expire_time < nowtime && level>=1) {//���Եȼ�����1�����
				printf("The request packet with URL %s expired and the record has been discarded\n",
				       trans_table[i].url);
			}
			//���и���
			trans_table[i].last_ID = id;
			trans_table[i].client = cli;
			trans_table[i].done = false;
			trans_table[i].expire_time = nowtime + ttl;
			memcpy(trans_table[i].url, url, sizeof(trans_table[i].url));
			return i;//���ص�ǰ���ڵ�index��Ϊid
		}
	}
	if (trans_count < ID_TRANS_SIZE) { //������е�����С��32��ֱ�����ӽ�
		trans_table[trans_count].last_ID = id;
		trans_table[trans_count].client = cli;
		trans_table[trans_count].done = false;
		trans_table[trans_count].expire_time = nowtime + ttl;
		memcpy(trans_table[i].url, url, sizeof(trans_table[i].url));
		trans_count++;
		return trans_count - 1; //������IDת���������˳��
	} else
		return -1;
}
