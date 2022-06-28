#include "total.h"

extern int level;
extern ID_TRANS_CELL trans_table[ID_TRANS_SIZE]; /*The ID_TRANS_TABLE*/
/*The cells of trans_table now*/
extern char fileName[129];

void Read_scheurl(DNIPList **local_dniplist, DNIPList **extern_dniplist)
{
	FILE *fp;

	int count = 0;//Used to record nodes number
	DNIPList *last;// Pointer to tail
#if _WIN64
	fopen_s(&fp, fileName, "r"); // read-only pattern
#elif __linux__
	int fdint = open(fileName, O_RDONLY);
	fp = fdopen(fdint, "r"); 
#endif
	if (!fp) {// read error
		printf("Read scheduled URL failed.\n"); 
		return;
	}
	char url[129], ip[16];
	*local_dniplist = (DNIPList *)malloc(sizeof(DNIPList));
	// initialization
	if (!*local_dniplist) { // malloc error
		return;
	}
	memset((*local_dniplist)->dn, 0, sizeof((*local_dniplist)->dn));
	memset((*local_dniplist)->ip, 0, sizeof((*local_dniplist)->ip));
	(*local_dniplist)->expire_time = -1;
	(*local_dniplist)->nextPtr = NULL;
	(*local_dniplist)->length = 1; // the length of nodes including the head node
	last = *local_dniplist; // initialize the last pointer 
	while (!feof(fp)) {
		fscanf(fp, "%s %s", ip, url);
		count++;
		DNIPList *temp = (DNIPList *)malloc(sizeof(DNIPList));
		if (!temp) { 
			return;
		}
		// initialize the node information
		memset(temp->dn, 0, sizeof(temp->dn));// domain
		memset(temp->ip, 0, sizeof(temp->ip));// ip
		(*local_dniplist)->expire_time = -1;// expire time
		temp->nextPtr = NULL;
		if (level >= 1) {
			printf("Read from 'dest.txt'->[URL:%s, IP:%s]\n", url,
			       ip);
		}
		// assign the value 
		memcpy(temp->ip, ip, sizeof ip); 
		memcpy(temp->dn, url, sizeof url);
		temp->expire_time = INFINITY; // local data never expire
		last->nextPtr = temp;
		last = temp; // update the tail location
		(*local_dniplist)->length++;
	}
	fclose(fp);
	printf("%d data have been saved\n", count);
	DNIPList *temp = (*local_dniplist)->nextPtr;
	// print the local data 
	for (int i = 0; i < count; i++) {
		printf("Data %d, URL:%s -> IP:%s\n", i + 1, temp->dn, temp->ip);
		temp = temp->nextPtr;
	}
	// initialize the temporary data
	*extern_dniplist = (DNIPList *)malloc(sizeof(DNIPList));// the head node of temporary data
	memset((*extern_dniplist)->dn, 0, sizeof((*extern_dniplist)->dn));
	memset((*extern_dniplist)->ip, 0, sizeof((*extern_dniplist)->ip));
	(*extern_dniplist)->expire_time = -1;
	(*extern_dniplist)->nextPtr = NULL;
	(*extern_dniplist)->length = 1;
	last->nextPtr = *extern_dniplist;//append to the local data
	return;
}

void print_team_msg()
{
	printf("*****************************************************************\n");
	printf("* @Course Name: Course Design of Computer Network               *\n");
	printf("* @Name of Team members: Yang Yexuan, Yang Yuheng, Weng Yuechuan*\n");
	printf("* @Teacher: Jiang Yanjun         @Class number: 2020211301      *\n");
	printf("* ------------------------------------------------------------- *\n");
	printf("*               DNS Relay Server - Ver 1.0                      *\n");
	printf("*****************************************************************\n");
	printf("Command syntax : MyDns [-d | -D] [dns-server-IP-addr]  \n"			);
}