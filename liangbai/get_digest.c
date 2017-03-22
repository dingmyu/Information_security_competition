#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sm3.h"
#include <malloc.h>
int main( int argc, char *argv[] )
{
        FILE *infp,*outfp;
        unsigned char input[200],output[200];
	int ilen = 0,i=0;
        infp=fopen("message.txt","r");
        outfp=fopen("digest.txt","w");
        fscanf(infp,"%s",input);
        ilen=strlen(input);
	sm3_context ctx;
	printf("Message:\n");
	printf("%s\n",input);
	sm3(input, ilen, output);
	printf("Hash:\n   ");
	for(i=0; i<32; i++)
	{
		fprintf(outfp,"\\x%02x",output[i]);
		printf("\\x%02x",output[i]);
		if (((i+1) % 4 ) == 0) printf(" ");
	} 
        fclose(infp);
	fclose(outfp);
        return 0;
}
