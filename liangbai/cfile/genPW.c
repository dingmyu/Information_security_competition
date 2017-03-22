#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sm3.h"

int main( int argc, char *argv[] )
{
        
        unsigned char *input=NULL,mainkey[]="mainkey";
	int ilen = 0,i=0;
	unsigned char output[32] = "";
        unsigned char appid[20] = "";
        unsigned char id[20] = "";
        unsigned char number[16] = "";
        input=(char*)malloc(200*sizeof(char));
        scanf("%s%s%s", appid, id, number);
        strcat(input,mainkey);
        strcat(input, appid);
        strcat(input, id);
        strcat(input,number);
        ilen=strlen(input);
	sm3_context ctx;
//	printf("%s\n",input);
	sm3(input, ilen, output);
        int flag = 0;
    char cipher[21] = "";
    for(int i = 0; i < 10; i++) {
        int tmp = output[i];
        if(tmp / 16 < 10)
            cipher[i * 2] = tmp / 16 + '0';
        
        else {
            flag++;
            if(flag % 2)
                cipher[i * 2] = tmp / 16 - 10 + 'A';
            else
                cipher[i * 2] = tmp / 16 - 10 + 'a';
        }

        if(tmp % 16 < 10)
            cipher[i * 2 + 1] = tmp % 16 + '0';
        else {
            flag++;
            if(flag % 2)
                cipher[i * 2 + 1] = tmp % 16 - 10 + 'A';
            else
                cipher[i * 2 + 1] = tmp % 16 - 10 + 'a';
        } 
    }
    printf("%s\n", cipher);
        return 0;
}
