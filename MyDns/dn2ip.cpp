#include "total.h"
extern int level;

char *Ip_str(DNIPList **Local_DNIPList, DNIPList **Temp_DNIPList, char *url)
{
	char *ip_str = (char *)malloc(sizeof(char)*16);
	char *destName = url;
	DNIPList *cur_Pair_In_Local_List = (*Local_DNIPList)->nextPtr,
		 *cur_Pair_In_Temp_List = (*Temp_DNIPList)->nextPtr;
	while (cur_Pair_In_Local_List != *Temp_DNIPList || cur_Pair_In_Temp_List) {
		if (cur_Pair_In_Local_List != *Temp_DNIPList &&
		    cur_Pair_In_Local_List &&
		    strcmp(cur_Pair_In_Local_List->dn, destName) ==
			    0) { //find in local
			strcpy_s(ip_str, sizeof ip_str,
					 cur_Pair_In_Local_List->ip);
			if (level > 0) {
				printf("从本地对照表获取IP成功\n");
			}
			return ip_str;

		} else if (cur_Pair_In_Temp_List &&
			   strcmp(cur_Pair_In_Temp_List->dn,
				  destName) == 0) { //find in temp
			strcpy_s(ip_str, sizeof ip_str,
					 cur_Pair_In_Temp_List->ip);
			if (level > 0) {
				printf("从网络缓存表获取IP成功\n");
			}
			return ip_str;
		}
		if (cur_Pair_In_Local_List != *Temp_DNIPList) {
			cur_Pair_In_Local_List = cur_Pair_In_Local_List->nextPtr;
		}
		
		if (cur_Pair_In_Temp_List) {
			cur_Pair_In_Temp_List = cur_Pair_In_Temp_List->nextPtr;
		}
	}
	strcpy_s(ip_str, sizeof ip_str, "failed");
	return ip_str;
}
