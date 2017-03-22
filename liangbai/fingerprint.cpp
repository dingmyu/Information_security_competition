#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
unsigned char* Upchar(int, int);
void ChangePasswd(sqlite3*, char*, int);
void SynchrKey(sqlite3*, int);
char statickey[33];
int statictime;

//还差SendToPC未完成,还有数字签名的部分(同步密钥)
//调试时加入了每次看数据库内容,去除了每次的指纹识别环节
void UART0_Open_Set() {
 	int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
  	struct termios options;//tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    tcgetattr(fd, &options);//设置串口输入波特率和输出波特率
    cfsetispeed(&options, B57600);
    cfsetospeed(&options, B57600); //修改控制模式，保证程序不会占用串口
   	options.c_cflag |= CLOCAL;//修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;//设置数据位
    options.c_cflag &= ~CSIZE; //屏蔽其他标志位
    options.c_cflag |= CS8;//设置校验位
    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~INPCK;// 设置停止位 
    options.c_cflag &= ~CSTOPB; //修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~ (IXON | IXOFF | IXANY);//整了爸爸两天啊!!!//设置等待时间和最小接收字符
    options.c_cc[VTIME] = 0; //读取一个字符等待0*(1/10)s
    options.c_cc[VMIN] = 0; //读取字符的最少个数为0
    tcflush(fd, TCIOFLUSH);//激活配置 (将修改后的termios数据设置到串口中）
    if(tcsetattr(fd,TCSANOW,&options)) {
    	printf("初始化串口失败\n");
    	close(fd);
    } else {
    	printf("树莓派初始化串口中...");
    	sleep(1);
    	printf("成功\n");
    	close(fd);
    }
}

void UART1_Open_Set() {	//声明设置串口的结构体
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);  
    struct termios option;//先清空该结构体
    bzero( &option, sizeof(option));//    cfmakeraw()设置终端属性，就是设置termios结构中的各个参数。
    cfmakeraw(&option);//设置波特率
    cfsetispeed(&option, B9600);
    cfsetospeed(&option, B9600);//CLOCAL和CREAD分别用于本地连接和接受使能，因此，首先要通过位掩码的方式激活这两个选项。
    option.c_cflag |= CLOCAL | CREAD;//通过掩码设置数据位为8位
    option.c_cflag &= ~CSIZE;
    option.c_cflag |= CS8;//设置无奇偶校验
    option.c_cflag &= ~PARENB;//一位停止位
    option.c_cflag &= ~CSTOPB;
    option.c_oflag &= ~OPOST;
    option.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    option.c_iflag &= ~ (IXON | IXOFF | IXANY);// 可设置接收字符和等待时间，无特殊要求可以将其设置为0
    option.c_cc[VTIME] = 0;
    option.c_cc[VMIN] = 0; //最小接收字符数// 用于清空输入/输出缓冲区
    tcflush (fd, TCIOFLUSH);
    //完成配置后，可以使用以下函数激活串口设置
    if(tcsetattr(fd,TCSANOW,&option)) {
        printf("初始化串口失败\n");
        close(fd);
    } else {
        printf("Windows初始化串口中...");
        sleep(1);
        printf("成功\n");
        close(fd);
    }
}

int SendToPC(char* str){//树莓派向电脑端发送数据
	int tx = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    printf("向电脑发送%s\n", str);
    //write(tx, str, strlen(str) + 1);
    close(tx);
    return 0;
}

void Printst(unsigned char* a, int len) {	//辅助函数
    for(int i = 0; i < len; i++)
        printf("0x%02x ", a[i]);
    printf("\n");
}

int GenImg(int fd) {	//指纹函数1
    unsigned char RBuf[16] = "";
	unsigned char WBuf[12] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
    int n = write(fd, WBuf, 12);
    int m = 0;
	int allm = 0;
	int sum = 12;//读12位
    while(allm < sum){
    	m = read(fd, RBuf + allm, sum - allm);
	    allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    if (RBuf[9] == 0)
		return 1;
    else
    	return 0;
}

int Img2Tz(int fd, int num) {	//指纹函数2
    unsigned char RBuf[16] = "";
    unsigned char WBuf[13] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x04, 0x02, 0x00, 0x00, 0x07};
    WBuf[10] += num;
    WBuf[12] += num;
    int n = write(fd, WBuf, 13);
    int m = 0;
    int allm = 0;
    int tmp = 0;
    int sum = 12;//读12位
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    if (RBuf[9] == 0)
        return 1;
    else
        return 0;
}

int RegModel(int fd) {	//指纹函数3
    unsigned char RBuf[16] = "";
    unsigned char WBuf[12] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09};
    int n = write(fd, WBuf, 12);
    int m = 0;
    int allm = 0;
    int sum = 12;//读12位
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    if (RBuf[9] == 0)
    	return 1;
    else
        return 0;
}

unsigned char* Upchar(int fd, int num) {	//指纹函数4
    unsigned char RBuf[1024] = "";
    unsigned char WBuf[13] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x04, 0x08, 0x00, 0x00, 0x0d};
    WBuf[10] += num;
    WBuf[12] += num;
    int n = write(fd, WBuf, 13);
    int m = 0;
    int allm = 0;
    int sum = 568;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    unsigned char* Buf = (unsigned char*)malloc(sizeof(unsigned char) * 278);
    for(int i = 0; i < 278; i++)
    	Buf[i] = RBuf[12 + i];
    return Buf;
}

unsigned char* Upcharr(int fd, int num) {    //指纹函数4(打印结果)
    unsigned char RBuf[1024] = "";
    unsigned char WBuf[13] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x04, 0x08, 0x00, 0x00, 0x0d};
    WBuf[10] += num;
    WBuf[12] += num;
    int n = write(fd, WBuf, 13);
    int m = 0;
    int allm = 0;
    int sum = 568;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);

    //打印结果
    printf("Upchar()结果: \n");
    Printst(RBuf, sum);
    

    unsigned char* Buf = (unsigned char*)malloc(sizeof(unsigned char) * 278);
    for(int i = 0; i < 278; i++)
        Buf[i] = RBuf[12 + i];
    return Buf;
}

int Downchar(int fd, unsigned char* buf1, int num) {	//指纹函数5
    unsigned char RBuf[16] = "";
    unsigned char WBuf[569] = "";
    WBuf[0] = 0xef;
    WBuf[1] = 0x01;
    WBuf[2] = 0xff;
    WBuf[3] = 0xff;
    WBuf[4] = 0xff;
    WBuf[5] = 0xff;
    WBuf[6] = 0x01;
    WBuf[7] = 0x00;
    WBuf[8] = 0x04;
    WBuf[9] = 0x09;
    WBuf[10] = 0x00 + num;
    WBuf[11] = 0x00;
    WBuf[12] = 0x0e + num;
    for(int i = 0; i < 556; i++) {
    	WBuf[i + 13] = buf1[i];
    	if(i == 423)
    		WBuf[i + 13] += 6;
    }
    if(WBuf[568] >= 0xfa){
    	WBuf[567]++;
    	WBuf[568] -= 128;
    	WBuf[568] += 6;
    	WBuf[568] -= 128;
    } else
    	WBuf[568] += 6;
    int n = write(fd, WBuf, 569);
    int m = 0;
    int allm = 0;
    int sum = 12;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    return 1;
}

int Match(int fd) {		//指纹函数6
    unsigned char RBuf[256] = "";
    unsigned char WBuf[12] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x03, 0x03, 0x00, 0x07};
    int n = write(fd, WBuf, 12);
    int m = 0;
    int allm = 0;
    int sum = 14;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    Printst(RBuf + 10, 2);
    if(RBuf[11] == 0){
        Upcharr(fd, 1);
        Upcharr(fd, 2);
    }
    if (RBuf[9] == 0){
        printf("匹配成功!\n");
        return 1;
    }
    else
        return 0;
}

static int _sql_callback(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数fuzhu
    for(int i = 0; i < argc; i++)
        printf("%s = %s\n", szColName[i], argv[i] == 0 ? "NULL" : argv[i]);
    printf("\n");
    return 0;
}

static int _sql_getmess(void *tmp, int argc, char **argv, char **szColName) {	//数据库函数zhiwen
	/*//输出数据库内容(加密后特征)
	for(int i = 0; i < 2224; i++)
		printf("%c", argv[1][i]);
	printf("\n");
	for(int i = 0; i < 2224; i++)
		printf("%c", argv[2][i]);
	printf("\n");
	*/
	FILE * fp1 = fopen("en1.txt", "w");//打开输入文件
    for(int i = 0; i < 2224; i++)
    	fprintf(fp1, "%c", argv[1][i]);
    fclose(fp1);
    FILE * pp1 = popen("./de1 < en1.txt", "r"); //建立管道
    char buffe1[1113];
	fgets(buffe1, sizeof(buffe1), pp1);
	unsigned char buffer1[1113];
	for(int i = 0; i < 1113; i++)
		buffer1[i] = buffe1[i];
	pclose(pp1);
	remove("en1.txt");
    FILE * fp2 = fopen("en2.txt", "w");//打开输入文件
    for(int i = 0; i < 2224; i++)
    	fprintf(fp2, "%c", argv[2][i]);
    fclose(fp2);
    FILE * pp2 = popen("./de1 < en2.txt", "r"); //建立管道
    char buffe2[1113];
	fgets(buffe2, sizeof(buffe2), pp2);
	unsigned char buffer2[1113];
	for(int i = 0; i < 1113; i++)
		buffer2[i] = buffe2[i];
	pclose(pp2);
	remove("en2.txt");
    unsigned char* bufall1 = (unsigned char*)malloc(sizeof(unsigned char) * 556);
    unsigned char* bufall2 = (unsigned char*)malloc(sizeof(unsigned char) * 556);
    for(int i = 0; i < 556; i++){
        char tmp1 = buffer1[2 * i];
        char tmp2 = buffer1[2 * i + 1];
        char tmp3 = buffer2[2 * i];
        char tmp4 = buffer2[2 * i + 1];
        if(tmp1 <= '9' && tmp2 <= '9')
            bufall1[i] = (tmp1 - '0') * 16 + (tmp2 - '0');
        else if(tmp1 <= '9' && tmp2 >= 'a')
            bufall1[i] = (tmp1 - '0') * 16 + (tmp2 - 'a' + 10);
        else if(tmp1 >= 'a' && tmp2 <= '9')
            bufall1[i] = (tmp1 - 'a' + 10) * 16 + (tmp2 - '0');
        else
            bufall1[i] = (tmp1 - 'a' + 10) * 16 + (tmp2 - 'a' + 10);
        if(tmp3 <= '9' && tmp4 <= '9')
            bufall2[i] = (tmp3 - '0') * 16 + (tmp4 - '0');
        else if(tmp3 <= '9' && tmp4 >= 'a')
            bufall2[i] = (tmp3 - '0') * 16 + (tmp4 - 'a' + 10);
        else if(tmp3 >= 'a' && tmp4 <= '9')
            bufall2[i] = (tmp3 - 'a' + 10) * 16 + (tmp4 - '0');
        else
            bufall2[i] = (tmp3 - 'a' + 10) * 16 + (tmp4 - 'a' + 10);
    }
    int fd = *(int*)tmp;
    Downchar(fd, bufall1, 1);
    Downchar(fd, bufall2, 2);
    RegModel(fd);//这三行出的问题
    return 0;  
}

static int _sql_callsearch(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数liechu
    printf("%s\n", argv[2] == 0 ? "NULL" : argv[2]);
    return 0;
}

static int _sql_callfuc2(void *tmp, int argc, char **argv, char **szColName) {  //辅助数据库函数xiugai
    if(argv[0][0] == '0') { //是旧口令(原始口令)
    	printf("旧密码(不用SM3的)\n");
    	FILE * fp1 = fopen("de2.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[1]); i++)
	    	fprintf(fp1, "%c", argv[1][i]);
	    fclose(fp1);
	    FILE * pp1 = popen("./de2 < de2.txt", "r"); //建立管道
	    char buffe1[100];
		fgets(buffe1, sizeof(buffe1), pp1);
		char buffer1[100];
		int i = 0;
		for(i = 0; i < 20 && buffe1[i]!=' '; i++)
			buffer1[i] = buffe1[i];
		buffer1[i--] = '\0';
		pclose(pp1);
      SendToPC(buffer1);//打印出来
		remove("de2.txt");
		printf("新密码(要用SM3的)\n");
		char* pswdd = (char*)tmp;
		FILE * fp2 = fopen("sm3.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[2]); i++)
	    	fprintf(fp2, "%c", argv[2][i]);
	    fprintf(fp2, " ");
	    for(int i = 0; i < strlen(argv[3]); i++)
	    	fprintf(fp2, "%c", argv[3][i]);
	    fprintf(fp2, " ");
	    for(int i = 0; i < strlen(pswdd); i++)
	    	fprintf(fp2, "%c", pswdd[i]);
	    fclose(fp2);
	    FILE * pp2 = popen("./sm3 < sm3.txt", "r"); //建立管道
	    char buffe2[100];
		fgets(buffe2, sizeof(buffe2), pp2);
		char buffer2[100];
		for(int i = 0; i < strlen(buffe2); i++)
			buffer2[i] = buffe2[i];
		buffer2[20] = '\0';
		pclose(pp2);
      SendToPC(buffer2);//打印出来
		remove("sm3.txt");
    } else {
    	printf("旧密码(要用SM3的)\n");
    	FILE * fp3 = fopen("sm31.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[2]); i++)
	    	fprintf(fp3, "%c", argv[2][i]);
	    fprintf(fp3, " ");
	    for(int i = 0; i < strlen(argv[3]); i++)
	    	fprintf(fp3, "%c", argv[3][i]);
	    fprintf(fp3, " ");
	    for(int i = 0; i < strlen(argv[4]); i++)
	    	fprintf(fp3, "%c", argv[4][i]);
	    fclose(fp3);
	    FILE * pp3 = popen("./sm3 < sm31.txt", "r"); //建立管道
	    char buffe3[100];
		fgets(buffe3, sizeof(buffe3), pp3);
		char buffer3[100];
		for(int i = 0; i < strlen(buffe3); i++)
			buffer3[i] = buffe3[i];
		buffer3[20] = '\0';
		pclose(pp3);
      printf("%s\n", buffer3);//打印出来
		remove("sm31.txt");
		printf("新密码(要用SM3的)\n");
		char* pswdd = (char*)tmp;
    	printf("%s\n", pswdd);//随机数
		FILE * fp4 = fopen("sm32.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[2]); i++)
	    	fprintf(fp4, "%c", argv[2][i]);
	    fprintf(fp4, " ");
	    for(int i = 0; i < strlen(argv[3]); i++)
	    	fprintf(fp4, "%c", argv[3][i]);
	    fprintf(fp4, " ");
	    for(int i = 0; i < strlen(pswdd); i++)
	    	fprintf(fp4, "%c", pswdd[i]);
	    fclose(fp4);
	    FILE * pp4 = popen("./sm3 < sm32.txt", "r"); //建立管道
	    char buffe4[100];
		fgets(buffe4, sizeof(buffe4), pp4);
		char buffer4[100];
		for(int i = 0; i < strlen(buffe4); i++)
			buffer4[i] = buffe4[i];
		buffer4[20] = '\0';
		pclose(pp4);
      printf("%s\n", buffer4);//打印出来
		remove("sm32.txt");
    }
    return 0;
}

static int _sql_callfuc4(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数fasong
    if(argv[0][0] == '0') { //是旧口令(原始口令)
    	printf("发送密码(不是SM3)\n");
    	FILE * fp1 = fopen("de2.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[1]); i++)
	    	fprintf(fp1, "%c", argv[1][i]);
	    fclose(fp1);
	    FILE * pp1 = popen("./de2 < de2.txt", "r"); //建立管道
	    char buffe1[100];
		fgets(buffe1, sizeof(buffe1), pp1);
		char buffer1[100];
		int i = 0;
		for(i = 0; i < 20 && buffe1[i]!=' '; i++)
			buffer1[i] = buffe1[i];
		buffer1[i--] = '\0';
		pclose(pp1);
      SendToPC(buffer1);//打印出来
		remove("de2.txt");
    } else {
    	printf("发送密码(用SM3的)\n");
    	FILE * fp3 = fopen("sm31.txt", "w");//打开输入文件
	    for(int i = 0; i < strlen(argv[2]); i++)
	    	fprintf(fp3, "%c", argv[2][i]);
	    fprintf(fp3, " ");
	    for(int i = 0; i < strlen(argv[3]); i++)
	    	fprintf(fp3, "%c", argv[3][i]);
	    fprintf(fp3, " ");
	    for(int i = 0; i < strlen(argv[4]); i++)
	    	fprintf(fp3, "%c", argv[4][i]);
	    fclose(fp3);
	    FILE * pp3 = popen("./sm3 < sm31.txt", "r"); //建立管道
	    char buffe3[100];
		fgets(buffe3, sizeof(buffe3), pp3);
		char buffer3[100];
		for(int i = 0; i < strlen(buffe3); i++)
			buffer3[i] = buffe3[i];
		buffer3[20] = '\0';
		pclose(pp3);
      printf("%s\n", buffer3);//打印出来
		remove("sm31.txt");
    }
    return 0;
}

void Initall() {
    UART0_Open_Set();
    UART1_Open_Set();
    sqlite3* db = 0;
    remove("main.db");
    char *pErrMsg = 0;  
    char *sSQL1 = "create table users(userid char(1) PRIMARY KEY, tz1 char(556), tz2 char(556));";
    char *sSQL2 = "create table accounts(boolean char(1), passwd char(20), appid char(20) PRIMARY KEY, id char(20), random char(15));";
    int ret = 0;  //连接数据库  
    ret = sqlite3_open("./main.db", &db);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "初始化数据库失败\n");  
        sqlite3_close(db);  
    }
    ret = sqlite3_exec(db, sSQL1, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);
        sqlite3_close(db);  
    } 
    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);
        sqlite3_close(db);  
    } 
    printf("初始化数据库中...");
    sleep(1);
    printf("成功\n");  
    sqlite3_close(db);  
    db = 0;
}

void Empty(int fd) {	//指纹函数7
    unsigned char RBuf[16] = "";
    unsigned char WBuf[569] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x04, 0x09, 0x01, 0x00, 0x0f, 
        0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x84, 0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x82, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x08, 0x00, 
        0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a};
    int n = write(fd, WBuf, 569);
    int m = 0;
    int allm = 0;
    int sum = 12;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    WBuf[10]++;
    WBuf[12]++;
    n = write(fd, WBuf, 569);
    m = 0;
    allm = 0;
    while(allm < sum){
        m = read(fd, RBuf + allm, sum - allm);
        allm += m;
    }
    tcflush(fd, TCIOFLUSH);
    RegModel(fd);
}

void Maininit() { //辅助函数
    printf("欢迎使用！初次使用时，请按任意键添加用户");
    char ch;
    scanf("%c", &ch);
}

int EnterFingerprint(int fd) {
	system("clear");
    printf("-----添加用户-----\n");
    printf("请按指纹\n");
    while(1) {
        int flag0 = GenImg(fd); //flag0 = 1录入成功
        if(flag0) {
            sleep(1);
            if(!Img2Tz(fd, 1)){//
                printf("读取失败，请重新按指纹\n");
                continue;
            } else {
                printf("读取成功！请再次按指纹\n");
                while(1) {
                    int flag1 = GenImg(fd); //flag0 = 1录入成功
                    if(flag1) {
                        sleep(1);
                        if(!Img2Tz(fd, 2)){
                            printf("读取失败，请重新按指纹\n");
                            continue;
                        } else if(!RegModel(fd)) {
                            printf("请按相同的手指\n");
                        }
                        else {
                            printf("绑定成功\n");
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    sleep(1);
    return 0;
}

void StoreDB(int fd, sqlite3* db) {
    unsigned char* buf1 = (unsigned char*)malloc(sizeof(unsigned char) * 278);
    buf1 = Upchar(fd, 1);
    unsigned char* buf2 = (unsigned char*)malloc(sizeof(unsigned char) * 278);
    buf2 = Upchar(fd, 2);
    unsigned char* bufall1 = (unsigned char*)malloc(sizeof(unsigned char) * 1112);
    unsigned char* bufall2 = (unsigned char*)malloc(sizeof(unsigned char) * 1112);
    for(int i = 0; i < 139; i++) {
        int tmp1 = buf1[i];
        int tmp2 = buf1[139 + i];
        int tmp3 = buf2[i];
        int tmp4 = buf2[139 + i];
        if(tmp1 / 16 < 10) {
            bufall1[278 + i * 2] = tmp1 / 16 + '0';
            bufall2[834 + i * 2] = tmp1 / 16 + '0';
        }
        else {
            bufall1[278 + i * 2] = tmp1 / 16 - 10 + 'a';
            bufall2[834 + i * 2] = tmp1 / 16 - 10 + 'a';
        }
        if(tmp1 % 16 < 10) {
            bufall1[279 + i * 2] = tmp1 % 16 + '0';
            bufall2[835 + i * 2] = tmp1 % 16 + '0';
        }
        else {
            bufall1[279 + i * 2] = tmp1 % 16 - 10 + 'a';
            bufall2[835 + i * 2] = tmp1 % 16 - 10 + 'a';
        }
        if(tmp2 / 16 < 10) {
            bufall1[556 + i * 2] = tmp2 / 16 + '0';
            bufall2[i * 2] = tmp2 / 16 + '0';
        }
        else {
            bufall1[556 + i * 2] = tmp2 / 16 - 10 + 'a';
            bufall2[i * 2] = tmp2 / 16 - 10 + 'a';
        }
        if(tmp2 % 16 < 10) {
            bufall1[557 + i * 2] = tmp2 % 16 + '0';
            bufall2[1 + i * 2] = tmp2 % 16 + '0';
        }
        else {
            bufall1[557 + i * 2] = tmp2 % 16 - 10 + 'a';
            bufall2[1 + i * 2] = tmp2 % 16 - 10 + 'a';
        }
        if(tmp3 / 16 < 10) {
            bufall1[834 + i * 2] = tmp3 / 16 + '0';
            bufall2[278 + i * 2] = tmp3 / 16 + '0';
        }
        else {
            bufall1[834 + i * 2] = tmp3 / 16 - 10 + 'a';
            bufall2[278 + i * 2] = tmp3 / 16 - 10 + 'a';
        }
        if(tmp3 % 16 < 10) {
            bufall1[835 + i * 2] = tmp3 % 16 + '0';
            bufall2[279 + i * 2] = tmp3 % 16 + '0';
        }
        else {
            bufall1[835 + i * 2] = tmp3 % 16 - 10 + 'a';
            bufall2[279 + i * 2] = tmp3 % 16 - 10 + 'a';
        }
        if(tmp4 / 16 < 10) {
            bufall1[i * 2] = tmp4 / 16 + '0';
            bufall2[556 + i * 2] = tmp4 / 16 + '0';
        }
        else {
            bufall1[i * 2] = tmp4 / 16 - 10 + 'a';
            bufall2[556 + i * 2] = tmp4 / 16 - 10 + 'a';
        }
        if(tmp4 % 16 < 10) {
            bufall1[1 + i * 2] = tmp4 % 16 + '0';
            bufall2[557 + i * 2] = tmp4 % 16 + '0';
        }
        else {
            bufall1[1 + i * 2] = tmp4 % 16 - 10 + 'a';
            bufall2[557 + i * 2] = tmp4 % 16 - 10 + 'a';
        }
    }//加密bufall1和bufall2是加密之前的buffer1和buffer2是加密之后的
    FILE * fp1 = fopen("en1.txt", "w");//打开输入文件
    for(int i = 0; i < 1112; i++)
    	fprintf(fp1, "%c", bufall1[i]);
    fclose(fp1);
    FILE * pp1 = popen("./en1 < en1.txt", "r"); //建立管道
    char buffe1[2225];
	fgets(buffe1, sizeof(buffe1), pp1);
	unsigned char buffer1[2225];
	for(int i = 0; i < 2225; i++)
		buffer1[i] = buffe1[i];
	pclose(pp1);
	remove("en1.txt");
    FILE * fp2 = fopen("en2.txt", "w");//打开输入文件
    for(int i = 0; i < 1112; i++)
    	fprintf(fp2, "%c", bufall2[i]);
    fclose(fp2);
    FILE * pp2 = popen("./en1 < en2.txt", "r"); //建立管道
    char buffe2[2225];
	fgets(buffe2, sizeof(buffe2), pp2);
	unsigned char buffer2[2225];
	for(int i = 0; i < 2225; i++)
		buffer2[i] = buffe2[i];
	pclose(pp2);
	remove("en2.txt");
    char *sSQL21 = "insert into users values('1', '";
    char *sSQL22 = "', '";
    char *sSQL23 = "');"; 
    char *sSQL2 = (char*)malloc(sizeof(char) * 4486);
    for(int i = 0; i < 31; i++)
        sSQL2[i] = sSQL21[i];
    for(int i = 0; i < 2224; i++)
        sSQL2[i + 31] = buffer1[i];
    for(int i = 0; i < 4; i++)
        sSQL2[i + 2255] = sSQL22[i];
    for(int i = 0; i < 2224; i++)
        sSQL2[i + 2259] = buffer2[i];
    for(int i = 0; i < 3; i++)
        sSQL2[i + 4483] = sSQL23[i];//构造输入字符串
    char *pErrMsg = 0;  
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    }
    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL insert error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);
        sqlite3_close(db);  
    }//需要加密
    Empty(fd);
}

void find(sqlite3* db, int fd) {
    char *sSQL3 = "select * from users;";  
    char *pErrMsg = 0;  
    int ret = 0;  //连接数据库 
    ret = sqlite3_open("./main.db", &db);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    } 
    int* tmp = &fd;
    ret = sqlite3_exec(db, sSQL3, _sql_getmess, (void*)tmp, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    } 
    sqlite3_close(db);  
    db = 0;
}

int Verify(int fd) {
	system("clear");
    printf("-----身份验证-----\n");
    printf("请按指纹\n");
    while(1) {
        //printf("1\n");
        int flag0 = GenImg(fd); //flag0 = 1录入成功
        //printf("2\n");
        if(flag0) {
            sleep(1);
            if(!Img2Tz(fd, 2)){
                printf("读取失败，请重新按指纹\n");
                continue;
            } else {
                if(Match(fd))
                    break;
                else
                    printf("身份验证失败，请重新按指纹\n");
            }
        }
    }
    Empty(fd);
    sleep(1);
    return 0;
}  

void AddAccount(sqlite3* db, int fd) {	//功能函数1(已完成)
	system("clear");
	printf("-----1.添加账号-----\n");
    char appid[20] = "";
    char id[20] = "";
    char passwd[21] = "";
    char passwdagain[21] = "";
    int flag = 0;
    while(!flag) {
    	printf("请输入APPID(提示:输入重复APPID将不会再存入):\n");
    	scanf("%s", appid);
    	if(appid[19]) {
    		printf("APPID过长,请重新输入APPID(不要到20字符)\n");
    		for(int i = 0; i < 20; i++)
    			appid[i] = 0;
    	} else
    		flag = 1;
    }
    flag = 0;
    while(!flag) {
    	printf("请输入ID:\n");
    	scanf("%s", id);
    	if(id[19]) {
    		printf("ID过长,请重新输入ID(不要到20字符)\n");
    		for(int i = 0; i < 20; i++)
    			id[i] = 0;
    	} else
    		flag = 1;
    }
    flag = 0;
    while(!flag) {
    	printf("请输入旧口令:\n");
    	scanf("%s", passwd);
    	if(passwd[20]) {
    		printf("旧口令过长,请重新输入旧口令或者先去更改旧口令再来输入(不要到20字符)\n");
    		for(int i = 0; i < 21; i++)
    			passwd[i] = 0;
    	} else
    		flag = 1;
    }
    flag = 0;
    while(!flag) {
    	printf("请再次输入旧口令:\n");
    	scanf("%s", passwdagain);
    	if(strcmp(passwd, passwdagain) != 0) {
    		printf("两次输入不一致,请重新输入\n");
    		for(int i = 0; i < 21; i++)
    			passwdagain[i] = 0;
    	} else
    		flag = 1;
    }

    int lena = strlen(appid);
    int lenb = strlen(id);
    int lenc = strlen(passwd);
	FILE * fp1 = fopen("en2.txt", "w");//打开输入文件
    for(int i = 0; i < lenc; i++)
    	fprintf(fp1, "%c", passwd[i]);
    for(int i = lenc; i < 20; i++)
    	fprintf(fp1, " ");
    fclose(fp1);
    FILE * pp1 = popen("./en2 < en2.txt", "r"); //建立管道
    char buffe1[41];
	fgets(buffe1, sizeof(buffe1), pp1);
	unsigned char buffer1[41];
	for(int i = 0; i < 41; i++)
		buffer1[i] = buffe1[i];
	pclose(pp1);
	//printf("%s\n", buffer1);//buffer1是加密过内容
	remove("en2.txt");    
    printf("提示:添加成功!是否修改为复杂口令?(y/n)\n");
    getchar();//我也不知道为什么要有这个getchar,反正就要有
    char note;
    scanf("%c", &note);
    if(note == 'y') {//修改口令
		char *sSQL21 = "insert or ignore into accounts values('0', '";
		char *sSQL22 = "', '";
		char *sSQL23 = "', '0');";
		char *sSQL2 = (char*)malloc(sizeof(char) * (101 + lena + lenb));
		for(int i = 0; i < 44; i++)
        	sSQL2[i] = sSQL21[i];
        for(int i = 0; i < 40; i++)
        	sSQL2[i + 44] = buffer1[i];
        for(int i = 0; i < 4; i++)
	        sSQL2[i + 84] = sSQL22[i];
	    for(int i = 0; i < lena; i++)
	        sSQL2[i + 88] = appid[i];
	    for(int i = 0; i < 4; i++)
	        sSQL2[i + 88 + lena] = sSQL22[i];
	    for(int i = 0; i < lenb; i++)
	        sSQL2[i + 92 + lena] = id[i];
	    for(int i = 0; i < 8; i++)
	        sSQL2[i + 92 + lena + lenb] = sSQL23[i];//构造输入字符串
	    sSQL2[100 + lena + lenb] = '\0';
	    //printf("%s\n", sSQL2);//输出构造字符串
	    char *pErrMsg = 0;  
	    int ret = 0;
	    ret = sqlite3_open("./main.db", &db);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
	        sqlite3_close(db);  
	    }
	    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL insert error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);
	        sqlite3_close(db);  
	    }//需要加密
	    sqlite3_close(db);  
	    db = 0;
	    printf("现在跳转到修改口令...\n");
        sleep(1);
        char *appi = (char*)malloc(sizeof(char) * (20));
        for(int i = 0; i < 20; i++)
        	appi[i] = appid[i];
	    ChangePasswd(db, appi, fd);
	}
    else{
		char *sSQL21 = "insert or ignore into accounts values('0', '";
		char *sSQL22 = "', '";
		char *sSQL23 = "', '0');";
		char *sSQL2 = (char*)malloc(sizeof(char) * (101 + lena + lenb));
		for(int i = 0; i < 44; i++)
        	sSQL2[i] = sSQL21[i];
        for(int i = 0; i < 40; i++)
	        sSQL2[i + 44] = buffer1[i];
	    for(int i = 0; i < 4; i++)
	        sSQL2[i + 84] = sSQL22[i];
	    for(int i = 0; i < lena; i++)
	        sSQL2[i + 88] = appid[i];
	    for(int i = 0; i < 4; i++)
	        sSQL2[i + 88 + lena] = sSQL22[i];
	    for(int i = 0; i < lenb; i++)
	        sSQL2[i + 92 + lena] = id[i];
	    for(int i = 0; i < 8; i++)
	        sSQL2[i + 92 + lena + lenb] = sSQL23[i];//构造输入字符串
	    sSQL2[100 + lena + lenb] = '\0';
	    //printf("%s\n", sSQL2);//输出构造字符串
	    char *pErrMsg = 0;  
	    int ret = 0;
	    ret = sqlite3_open("./main.db", &db);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
	        sqlite3_close(db);  
	    }
	    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL insert error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);
	        sqlite3_close(db);  
	    }//需要加密
	    sqlite3_close(db);  
	    db = 0;
	    printf("口令信息存储成功!\n");
	    sleep(1);
	}
}

void ChangePasswd(sqlite3* db, char* appi, int fd) {	//功能函数2(已完成)
	system("clear");
	printf("-----2.修改口令-----\n");
	char *pErrMsg = 0;  
	int ret = 0;
	char appid[20] = "";
	ret = sqlite3_open("./main.db", &db);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
	        sqlite3_close(db);  
	    }
	if(appi == NULL) {
	    char *sSQL3 = "select * from accounts;";  
	    printf("请选择想要修改口令的账户:\n");
	    ret = sqlite3_exec(db, sSQL3, _sql_callsearch, 0, &pErrMsg);  //这里调用了callsearch
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);  
	        sqlite3_close(db);  
	    } 
    	scanf("%s", appid);
	}
	else
		for(int i = 0; i < 20; i++)
			appid[i] = appi[i];
	getchar();
	time_t t;
	if(time(&t) - statictime > 300) {
		printf("秘钥已经过期,请先同步密钥\n");
		sleep(2);
		SynchrKey(db, fd);
	} else {
		char *sSQL21 = "select * from accounts where appid = '";
	    char *sSQL23 = "';";
	    int lena = strlen(appid);
		char *sSQL2 = (char*)malloc(sizeof(char) * (41 + lena));
		for(int i = 0; i < 38; i++)
	    	sSQL2[i] = sSQL21[i];
	    for(int i = 0; i < lena; i++)
	    	sSQL2[i + 38] = appid[i];
	    for(int i = 0; i < 2; i++)
	        sSQL2[i + 38 + lena] = sSQL23[i];//构造输入字符串
	    sSQL2[40 + lena] = '\0';
	    srand((int) time(0));
	    char *num = (char*)malloc(sizeof(char) * 15);
	    sprintf(num, "%d", rand());//获得随机数
	    ret = sqlite3_exec(db, sSQL2, _sql_callfuc2, (void*)num, &pErrMsg);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);  
	        sqlite3_close(db);  
	    } 
	    int lend = strlen(num);
		char *sSQL11 = "update accounts set boolean = '1', passwd = ' ', random = '";
		char *sSQL12 = "' where appid = '";
		char *sSQL13 = "';";
		char *sSQL1 = (char*)malloc(sizeof(char) * (79 + lend + lena));
		for(int i = 0; i < 59; i++)
			sSQL1[i] = sSQL11[i];
		for(int i = 0; i < lend; i++)
			sSQL1[i + 59] = num[i];
		for(int i = 0; i < 17; i++)
			sSQL1[i + 59 + lend] = sSQL12[i];
		for(int i = 0; i < lena; i++)
			sSQL1[i + 76 + lend] = appid[i];
		for(int i = 0; i < 2; i++)
			sSQL1[i + 76 + lend + lena] = sSQL23[i];
		sSQL1[78 + lend + lena] = '\0';
		ret = sqlite3_exec(db, sSQL1, _sql_callback, 0, &pErrMsg);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);  
	        sqlite3_close(db);  
	    } 
	    sqlite3_close(db);  
	    db = 0; 
	}
	sleep(1);
}

void SynchrKey(sqlite3* db, int fd) {	//功能函数3
	system("clear");
	printf("-----3.同步密钥-----\n");
	printf("请先进行身份验证(然而现在我们并不需要身份验证)\n");
	sleep(1);
	

  find(db, fd);
  Verify(fd);
  printf("-----3.同步密钥-----\n");
  //*/
	//这里需要与PC端通信,得到一个128bit的秘钥(32位16进制数组)
	char str[33] = "6162636465666768696a6b6c6d6e6f70";
	strcpy(statickey, str);
	printf("同步密钥成功!\n");
	time_t t;
	statictime = time(&t);
	sleep(1);
	//把str配置成en de的参数并编译
	//SendToPC(str);
}

void Login(sqlite3* db, int fd) {	//功能函数4(已完成)
	system("clear");
	printf("-----4.登录账号-----\n");
	char *pErrMsg = 0;  
	int ret = 0;
	char appid[20] = "";
	ret = sqlite3_open("./main.db", &db);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
	        sqlite3_close(db);  
	    }
    char *sSQL3 = "select * from accounts;";  
    printf("请选择想要修改口令的账户:\n");
    ret = sqlite3_exec(db, sSQL3, _sql_callsearch, 0, &pErrMsg);  //这里调用了callsearch
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    } 
	scanf("%s", appid);
	getchar();
	time_t t;
	if(time(&t) - statictime > 300) {
		printf("秘钥已经过期,请先同步密钥\n");
		sleep(2);
		SynchrKey(db, fd);
	} else {
		char *sSQL21 = "select * from accounts where appid = '";
	    char *sSQL23 = "';";
	    int lena = strlen(appid);
		char *sSQL2 = (char*)malloc(sizeof(char) * (41 + lena));
		for(int i = 0; i < 38; i++)
	    	sSQL2[i] = sSQL21[i];
	    for(int i = 0; i < lena; i++)
	    	sSQL2[i + 38] = appid[i];
	    for(int i = 0; i < 2; i++)
	        sSQL2[i + 38 + lena] = sSQL23[i];//构造输入字符串
	    sSQL2[40 + lena] = '\0';
	    ret = sqlite3_exec(db, sSQL2, _sql_callfuc4, 0, &pErrMsg);  
	    if (ret != SQLITE_OK) {  
	        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
	        sqlite3_free(pErrMsg);  
	        sqlite3_close(db);  
	    } 
	    sqlite3_close(db);  
	    db = 0; 
	}
}

void AccountDel(sqlite3* db) {	//功能函数5(已完成)
    char *pErrMsg = 0;  
	int ret = 0;
    ret = sqlite3_open("./main.db", &db);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    }
    char *sSQL3 = "select * from accounts;";  
    printf("请选择想要删除的账户:\n");
    ret = sqlite3_exec(db, sSQL3, _sql_callsearch, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    }  
    
    char appid[20];
    scanf("%s", appid);
    char *sSQL21 = "delete from accounts where appid = '";
    char *sSQL23 = "';";
    int lena = strlen(appid);	
	char *sSQL2 = (char*)malloc(sizeof(char) * (39 + lena));
	for(int i = 0; i < 36; i++)
    	sSQL2[i] = sSQL21[i];
    for(int i = 0; i < lena; i++)
    	sSQL2[i + 36] = appid[i];
    for(int i = 0; i < 2; i++)
        sSQL2[i + 36 + lena] = sSQL23[i];//构造输入字符串
    sSQL2[38 + lena] = '\0';
    ret = sqlite3_exec(db, sSQL2, _sql_callsearch, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    } 
    sqlite3_close(db);  
    db = 0; 
    printf("删除账号成功!\n");
    sleep(1);
}

void Reset(sqlite3* db) {		//功能函数6(已完成)
	system("clear");
	printf("-----恢复出厂设置-----\n");
	char *pErrMsg = 0;  
	int ret = 0;
	char *sSQL31 = "DROP TABLE users";
	char *sSQL32 = "DROP TABLE accounts";
    ret = sqlite3_open("./main.db", &db);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
    }
    ret = sqlite3_exec(db, sSQL31, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    }
    ret = sqlite3_exec(db, sSQL32, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);  
        sqlite3_close(db);  
    }
    char *sSQL1 = "create table users(userid char(1) PRIMARY KEY, tz1 char(556), tz2 char(556));";
    char *sSQL2 = "create table accounts(boolean char(1), passwd char(20), appid char(20) PRIMARY KEY, id char(20), random char(15));";
    ret = sqlite3_exec(db, sSQL1, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);
        sqlite3_close(db);  
    } 
    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);  
    if (ret != SQLITE_OK) {  
        fprintf(stderr, "SQL create error: %s\n", pErrMsg);  
        sqlite3_free(pErrMsg);
        sqlite3_close(db);  
    } 
    printf("初始化中...");
    sleep(1);
    printf("成功\n");  
    sqlite3_close(db); 
    db = 0;
    sleep(2);
}

int main() {
	system("clear");
    Initall();//放在别的程序里，每次连接上只打开一次
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);//打开端口
  Empty(fd);//初始化端口缓冲区，以防泄露数据信息

    /*//验证端口已经初始化
    printf("清空指纹端缓冲数据:\n");
    Upchar(fd, 1);
    Upchar(fd, 2);
    */

    int quit_flag = 0;
    while(!quit_flag) {
    	quit_flag = 1;
	    ///*//调试不需要
	  Maininit(); //显示界面，并没有太大卵用
	  EnterFingerprint(fd);//第一次录入指纹
	    sqlite3* db = 0;
	  StoreDB(fd, db);//把指纹消息存入数据库
		//*/
	    
	    /*//验证已经清零
	    printf("验证已经清空\n");
	    Upchar(fd, 1);
	    Upchar(fd, 2);
		*/

	    int command, exit_flag = 0;
	    while(!exit_flag) {

      find(db, fd);
	  Verify(fd);

	        system("clear");
	        time_t t;
	    	int time1 = time(&t);//当前系统时间(验证完时间)
	        printf("-----主界面-----\n  1:添加账号\n  2:修改口令\n  3:同步密钥\n  4:登录账号\n  5:删除账号\n  6:恢复出厂设置\n现在时间%d\n上次同步密钥时间%d\n", time(&t), statictime);
	        scanf("%d",&command);
	        int time2 = time(&t);
	        if(time2 - time1 >= 60) {//验证时间10s
	        	command = 0;
	        	printf("提示:身份验证时间已到,请重新认证\n");
	        	sleep(2);
	        }
	        switch(command) {
	            case 0://验证时间已到
	            	break;
	            case 1:
	                AddAccount(db, fd);
	                break;
	            case 2:
	            	ChangePasswd(db, NULL, fd);
	            	break;
	            case 3:
	            	SynchrKey(db, fd);
	            	break;
	            case 4:
	            	Login(db, fd);
	            	break;
	            case 5:
	            	AccountDel(db);
	            	break;
	            case 6://删除表
	            	quit_flag = 0;
	            	exit_flag = 1;
	            	Reset(db);	
	            	break;
	            default:
	                exit_flag = 1;
	                break;
	        }
	        //每回合结束检查数据库
	        printf("\n\n\n自动检查数据库内容:\n");
	        char *pErrMsg = 0;  
			int ret = 0;
		    ret = sqlite3_open("./main.db", &db);  
		    if (ret != SQLITE_OK) {  
		        fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));  
		        sqlite3_close(db);  
		    }
		    char *sSQL3 = "select * from accounts;";  
		    ret = sqlite3_exec(db, sSQL3, _sql_callback, 0, &pErrMsg);  
		    if (ret != SQLITE_OK) {  
		        fprintf(stderr, "SQL error: %s\n", pErrMsg);  
		        sqlite3_free(pErrMsg);  
		        sqlite3_close(db);  
		    } 
		    sqlite3_close(db);  
		    db = 0;       
	        printf("请按任意键继续\n");
	        getchar();
	        getchar();
	    }
	}
    close(fd);
    remove("main.db");
	return 0;
}