#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include "dialog.h"
#include <QElapsedTimer>
#include <QLabel>
//extern Ui::Dialog * Dialog::getUi();

unsigned char* Upchar(int, int);
void ChangePasswd(sqlite3*, char*, int);
void SynchrKey(sqlite3*, int);
char statickey[33];
int statictime = 0,fingertime = 0;
sqlite3* db = 0;
char delarry[100][100];
int deli = 0;

void mainwindow::judge(){
    time_t t;
    if(time(&t) - fingertime > 120) {
        int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
        Verify(fd);
        ::close(fd);
    }
}

void mainwindow::UART0_Open_Set() {
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
        ui->label->setText("初始化串口失败");
        ui->label->show();
        this->repaint();
        ::close(fd);
    } else {
        ui->label->setText("树莓派初始化串口中...");
        ui->label->show();
        this->repaint();
        usleep(20000);
        ui->label->setText("成功");
        ui->label->show();
        this->repaint();
        ::close(fd);
    }
}

void mainwindow::UART1_Open_Set() {	//声明设置串口的结构体
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    struct termios option;//先清空该结构体
//    bzero( &option, sizeof(option));//    cfmakeraw()设置终端属性，就是设置termios结构中的各个参数。
//    cfmakeraw(&option);//设置波特率
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
        ui->label->setText("初始化串口失败");
        ui->label->show();
        this->repaint();
        ::close(fd);
    } else {
        ui->label->setText("Windows初始化串口中...");
        ui->label->show();
        this->repaint();
        usleep(20000);
        ui->label->setText("成功");
        ui->label->show();
        this->repaint();
       ::close(fd);
    }
}


int send(int tx, char* str){
    tcflush(tx, TCIOFLUSH);
    //printf("发送%s\n", str);
    // if(write(tx, str, strlen(str) + 1)==-1)
    write(tx, str, 30);//==-1
//        printf("write failed, errno:%d\n",errno);
    //else printf("write %s\n",str);
    return 0;
}
/*
void SendToPC(char str[200]){//树莓派向电脑端发送数据
    int tx = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    char output[300] = "";
    char temstr[8];
    printf("开始向电脑发送%s\n", str);
    for(int i = 0; i < strlen(str); i+=30){
        sleep(2);
        send(tx, str + i);
    }
    sleep(2);
    send(tx,"|");
}

void signature2(char str[40]) {//数字签名部分(修改口令2)
    //str是密码
    FILE * fp0 = fopen("en4.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(statickey); i++)
        fprintf(fp0, "%c", statickey[i]);
    fprintf(fp0, "\n");
    for(int i = 0; i < strlen(str); i++)
        fprintf(fp0, "%c", str[i]);
    for(int i = 0; i < 20 - strlen(str); i++)
        fprintf(fp0, " ");
    fclose(fp0);
    FILE * pp1 = popen("./en4 < en4.txt", "r"); //建立管道
    char buffe1[41] = "";
    fgets(buffe1, sizeof(buffe1), pp1);
    char buffer1[41] = "";
    for(int i = 0; i < 41; i++)
        buffer1[i] = buffe1[i];
    buffer1[40] = '\0';
    pclose(pp1);
    remove("en4.txt");

    FILE * fp1 = fopen("message.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(buffer1); i++)
        fprintf(fp1, "%c", buffer1[i]);
    fclose(fp1);
    system("./get_digest");
    remove("message.txt");
    system("./sendA");
    remove("digest.txt");
    FILE *fp;
    fp = fopen("signature.txt","r");
    char signature[200] = "";
    fscanf(fp, "%s", signature);//数字签名得到

    printf("\n");

    char* bufall = (char*)malloc(sizeof(char) * (4 + strlen(signature) + strlen(buffer1)));
    bufall[0] = '3';
    bufall[1] = '|';
    for(int i = 0; i < strlen(signature); i++)
        bufall[2 + i] = signature[i];
    bufall[2 + strlen(signature)] = '|';

    for(int i = 0; i < strlen(buffer1); i++)
        bufall[3 + strlen(signature) + i] = buffer1[i];
    bufall[3 + strlen(signature) + strlen(buffer1)] = '|';

    SendToPC(bufall);

    SendToPC("2");
    SendToPC(signature);
    SendToPC(buffer1);
    qDebug()<<"2222"<<bufall;
    remove("signature.txt");
}

void signature4(char str[40], char id[50]) {//数字签名部分(登录账户4)
    //str是密码
    FILE * fp0 = fopen("en4.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(statickey); i++)
        fprintf(fp0, "%c", statickey[i]);
    fprintf(fp0, "\n");
    for(int i = 0; i < strlen(str); i++)
        fprintf(fp0, "%c", str[i]);
    for(int i = 0; i < 20 - strlen(str); i++)
        fprintf(fp0, " ");
    fclose(fp0);
    FILE * pp1 = popen("./en4 < en4.txt", "r"); //建立管道
    char buffe1[41] = "";
    fgets(buffe1, sizeof(buffe1), pp1);
    char buffer1[41] = "";
    for(int i = 0; i < 41; i++)
        buffer1[i] = buffe1[i];
    buffer1[40] = '\0';
    pclose(pp1);
    remove("en4.txt");

    FILE * fp1 = fopen("message.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(buffer1); i++)
        fprintf(fp1, "%c", buffer1[i]);
    fclose(fp1);
    system("./get_digest");
    remove("message.txt");
    system("./sendA");
    remove("digest.txt");
    FILE *fp;
    fp = fopen("signature.txt","r");
    char signature[200] = "";
    fscanf(fp, "%s", signature);
    printf("\n");

    char* bufall = (char*)malloc(sizeof(char) * (5 + strlen(signature) + strlen(id) + strlen(buffer1)));
    bufall[0] = '3';
    bufall[1] = '|';
    for(int i = 0; i < strlen(signature); i++)
        bufall[2 + i] = signature[i];
    bufall[2 + strlen(signature)] = '|';

    for(int i = 0; i < strlen(id); i++)
        bufall[3 + strlen(signature) + i] = id[i];
    bufall[3 + strlen(signature) + strlen(id)] = '|';

    for(int i = 0; i < strlen(buffer1); i++)
        bufall[4 + strlen(signature) + strlen(id) + i] = buffer1[i];
    bufall[4 + strlen(signature) + strlen(id) + strlen(buffer1)] = '|';

    SendToPC("3");
    SendToPC(signature);
    SendToPC(id);
    SendToPC(buffer1);
    qDebug()<<"4444"<<bufall;
    SendToPC(bufall);
    remove("signature.txt");
}
*/

void SendToPC(char str[200]){//树莓派向电脑端发送数据
    int tx = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    char output[300] = "";
    char temstr[8];
    printf("开始向电脑发送%s\n", str);
    for(int i = 0; i < strlen(str); i+=30){
        sleep(2);
        send(tx, str + i);
    }
    sleep(2);
    send(tx, "|");
}

void signature2(char str[40]) {//数字签名部分(修改口令2)
    //str是密码
    FILE * fp0 = fopen("en4.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(statickey); i++)
        fprintf(fp0, "%c", statickey[i]);
    fprintf(fp0, "\n");
    for(int i = 0; i < strlen(str); i++)
        fprintf(fp0, "%c", str[i]);
    for(int i = 0; i < 20 - strlen(str); i++)
        fprintf(fp0, " ");
    fclose(fp0);
    FILE * pp1 = popen("./en4 < en4.txt", "r"); //建立管道
    char buffe1[41] = "";
    fgets(buffe1, sizeof(buffe1), pp1);
    char buffer1[41] = "";
    for(int i = 0; i < 41; i++)
        buffer1[i] = buffe1[i];
    buffer1[40] = '\0';
    pclose(pp1);
    remove("en4.txt");

    FILE * fp1 = fopen("message.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(buffer1); i++)
        fprintf(fp1, "%c", buffer1[i]);
    fclose(fp1);
    system("./get_digest");
    remove("message.txt");
    system("./sendA");
    remove("digest.txt");
    FILE *fp;
    fp = fopen("signature.txt","r");
    char signature[200] = "";
    fscanf(fp, "%s", signature);//数字签名得到

    printf("\n");
    SendToPC("2");
    SendToPC(signature);
    SendToPC(buffer1);
    remove("signature.txt");
}

void signature4(char str[40], char id[50]) {//数字签名部分(登录账户4)
    //str是密码
    FILE * fp0 = fopen("en4.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(statickey); i++)
        fprintf(fp0, "%c", statickey[i]);
    fprintf(fp0, "\n");
    for(int i = 0; i < strlen(str); i++)
        fprintf(fp0, "%c", str[i]);
    for(int i = 0; i < 20 - strlen(str); i++)
        fprintf(fp0, " ");
    fclose(fp0);
    FILE * pp1 = popen("./en4 < en4.txt", "r"); //建立管道
    char buffe1[41] = "";
    fgets(buffe1, sizeof(buffe1), pp1);
    char buffer1[41] = "";
    for(int i = 0; i < 41; i++)
        buffer1[i] = buffe1[i];
    buffer1[40] = '\0';
    pclose(pp1);
    remove("en4.txt");

    FILE * fp1 = fopen("message.txt", "w");//打开输入文件
    for(int i = 0; i < strlen(buffer1); i++)
        fprintf(fp1, "%c", buffer1[i]);
    fclose(fp1);
    system("./get_digest");
    remove("message.txt");
    system("./sendA");
    remove("digest.txt");
    FILE *fp;
    fp = fopen("signature.txt","r");
    char signature[200] = "";
    fscanf(fp, "%s", signature);
    printf("\n");
    SendToPC("3");
    SendToPC(signature);
    SendToPC(id);
    SendToPC(buffer1);
    remove("signature.txt");
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

int mainwindow::Match(int fd) {		//指纹函数6
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
  /*  if(RBuf[11] == 0){
        ui->label->setText("匹配失败");
        ui->label->show();
        this->repaint();*/
 /*       QLabel *label = new QLabel("<h2><i>g</i>"
                                   "<font color=red>g</font></h2>");
        label -> show();
        label -> move(200,120);
        QElapsedTimer et;
        et.start();
        while(et.elapsed()<300)
            QCoreApplication::processEvents();
//        label -> repaint();*/
  /*      return -1;
    }*/
    if (RBuf[9] == 0){
        ui->label->setText("匹配成功");
        ui->label->show();
        this->repaint();
        return 1;
    }
    else
        return 0;
}

static int _sql_callback(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数
    for(int i = 0; i < argc; i++)
        printf("%s = %s\n", szColName[i], argv[i] == 0 ? "NULL" : argv[i]);
    printf("\n");
    return 0;
}

static int _sql_getmess(void *tmp, int argc, char **argv, char **szColName) {	//数据库函数
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

void mainwindow::insert(char *a){
    QString str = QString(QLatin1String(a));
    ui->deletecombo->addItem(str);
}

int _sql_callsearch(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数
/*    char *a;
    if(argv[2] != 0)  a = argv[2];
    QString str = QString(QLatin1String(a));
    ui->deletecombo->addItem(str);*/

/*    char *a;
    if(argv[2] != 0)  a = argv[2];
    strcpy(delarry[deli],a);
    deli++;
    qDebug()<<"2333"<<a;*/

    qDebug()<< "2333" <<argv[2];
//    ((mainwindow*)notused)->insert(a);
    return 0;
}

static int _sql_callfucold(void *tmp, int argc, char **argv, char **szColName) {  //辅助数据库函数
    if(argv[0][0] == '0') { //是旧口令(原始口令)
        qDebug()<< "旧密码(不用SM3的)";
        FILE * fp1 = fopen("de2.txt", "w");//打开输入文件
        for(int i = 0; i < strlen(argv[1]); i++)
            fprintf(fp1, "%c", argv[1][i]);
        fclose(fp1);
        FILE * pp1 = popen("./de2 < de2.txt", "r"); //建立管道
        char buffe1[21] = "";
        fgets(buffe1, sizeof(buffe1), pp1);
        char buffer1[21] = "";
        int i = 0;
        for(i = 0; i < 21 && buffe1[i]!=' '; i++)
            buffer1[i] = buffe1[i];
        buffer1[i--] = '\0';
        pclose(pp1);
        qDebug()<< buffer1;
        signature2(buffer1);//打印出来
        remove("de2.txt");
        qDebug()<<"old1 fawanle!";
        //getchar();
/*        ::sleep(30);
        qDebug()<< "新密码(要用SM3的";
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
        char buffe2[21] = "";
        fgets(buffe2, sizeof(buffe2), pp2);
        char buffer2[21] = "";
        for(int i = 0; i < strlen(buffe2); i++)
            buffer2[i] = buffe2[i];
        buffer2[21] = '\0';
        pclose(pp2);
        qDebug()<< buffer2;
        signature2(buffer2);//打印出来
        remove("sm3.txt");*/
    } else {
        qDebug()<< "旧密码(不用SM3的)";
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
        signature2(buffer3);//打印出来
        remove("sm31.txt");
        qDebug()<<"old2 fawanle!";
/*        ::sleep(30);
        qDebug()<< "新密码(要用SM3的";
        char* pswdd = (char*)tmp;
        qDebug()<< pswdd;
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
        signature2(buffer4);//打印出来
        remove("sm32.txt");*/
    }
    return 0;
}

static int _sql_callfucnew(void *tmp, int argc, char **argv, char **szColName) {  //辅助数据库函数
    if(argv[0][0] == '0') { //是旧口令(原始口令)
/*        qDebug()<< "旧密码(不用SM3的)";
        FILE * fp1 = fopen("de2.txt", "w");//打开输入文件
        for(int i = 0; i < strlen(argv[1]); i++)
            fprintf(fp1, "%c", argv[1][i]);
        fclose(fp1);
        FILE * pp1 = popen("./de2 < de2.txt", "r"); //建立管道
        char buffe1[21] = "";
        fgets(buffe1, sizeof(buffe1), pp1);
        char buffer1[21] = "";
        int i = 0;
        for(i = 0; i < 21 && buffe1[i]!=' '; i++)
            buffer1[i] = buffe1[i];
        buffer1[i--] = '\0';
        pclose(pp1);
        qDebug()<< buffer1;
        signature2(buffer1);//打印出来
        remove("de2.txt");
        //getchar();
        ::sleep(30);*/
        qDebug()<< "新密码(要用SM3的";
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
        char buffe2[21] = "";
        fgets(buffe2, sizeof(buffe2), pp2);
        char buffer2[21] = "";
        for(int i = 0; i < strlen(buffe2); i++)
            buffer2[i] = buffe2[i];
        buffer2[21] = '\0';
        pclose(pp2);
        qDebug()<< buffer2;
        signature2(buffer2);//打印出来
        remove("sm3.txt");
        qDebug()<<"new1 fawanle!";
    } else {
    /*    qDebug()<< "旧密码(不用SM3的)";
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
        signature2(buffer3);//打印出来
        remove("sm31.txt");
        ::sleep(30);*/
        qDebug()<< "新密码(要用SM3的";
        char* pswdd = (char*)tmp;
        qDebug()<< pswdd;
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
        signature2(buffer4);//打印出来
        remove("sm32.txt");
        qDebug()<<"new2 fawanle!";
    }
    return 0;
}

static int _sql_callfuc4(void *notused, int argc, char **argv, char **szColName) {  //辅助数据库函数
    if(argv[0][0] == '0') { //是旧口令(原始口令)
        qDebug()<< "旧密码(不用SM3的)";
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
        signature4(buffer1, argv[3]);//打印出来
        remove("de2.txt");
    } else {
        qDebug()<< "新密码(要用SM3的";
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
        signature4(buffer3, argv[3]);//打印出来
        remove("sm31.txt");
    }
    return 0;
}

void mainwindow::Initall() {
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
    ui->label->setText("初始化数据库中...");
    ui->label->show();
    this->repaint();
    usleep(200000);
    ui->label->setText("成功");
    ui->label->show();
    this->repaint();
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

int mainwindow::EnterFingerprint(int fd) {
    while(1) {
        int flag0 = GenImg(fd); //flag0 = 1录入成功
        if(flag0) {
            usleep(20000);
            if(!Img2Tz(fd, 1)){//
                ui->label->setText("读取失败，请重新按指纹");
                ui->label->show();
                this->repaint();
                continue;
            } else {
 /*               Dialog a;
                a.move(100,80);
                Ui::Dialog *ui1 = a.getUi();
         //       ui1->label->hide();
                a.show();*/
                ui->label->setText("读取成功！请再次按指纹");
                ui->label->show();
                this->repaint();
        /*        QElapsedTimer et;
                et.start();
                while(et.elapsed()<300)
                    QCoreApplication::processEvents();*/
                while(1) {
                    int flag1 = GenImg(fd); //flag0 = 1录入成功
                    if(flag1) {
                        usleep(20000);
                        if(!Img2Tz(fd, 2)){
                            ui->label->setText("读取失败，请重新按指纹");
                            ui->label->show();
                            this->repaint();
                            continue;
                        } else if(!RegModel(fd)) {
                            ui->label->setText("请按相同的手指");
                            ui->label->show();
                            this->repaint();
                        }
                        else {
                            ui->label->setText("绑定成功");
                            ui->label->show();
                            this->repaint();
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    usleep(20000);
//    return 0;
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
//    Empty(fd);
}

void mainwindow::find(sqlite3* db, int fd) {
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

int mainwindow::Verify(int fd) {
    ui->label->setText("请按指纹");
    ui->label->show();
    this->repaint();
    while(1) {
        int flag0 = GenImg(fd); //flag0 = 1录入成功
        if(flag0) {
            usleep(20000);
            if(!Img2Tz(fd, 2)){
                ui->label->setText("读取失败，请重新按指纹");
                ui->label->show();
                this->repaint();
                continue;
            } else {
              /*  if(Match(fd)==-1){
                    ui->label->setText("这次又失败了，gg");
                    ui->label->show();
                    this->repaint();
                    return -1;
                }
                else*/ if(Match(fd)==1){
                    time_t t;
                    fingertime = time(&t);
                    break;
                }
                else{
                    ui->label->setText("身份验证失败，请重新按指纹");
                    ui->label->show();
                    this->repaint();
                }
            }
        }
    }
//    Empty(fd);
    usleep(20000);
    return 0;
}

void mainwindow::AddAccount(sqlite3* db, int fd) {	//功能函数1(已完成)
/*    char appid[20] = "";
    char id[20] = "";
    char passwd[21] = "";
    char passwdagain[21] = "";*/
    QByteArray ba = mainwindow::appid.toLatin1();
    char *appid = ba.data();
    QByteArray ba1 = mainwindow::id.toLatin1();
    char *id = ba1.data();
    QByteArray ba2 = mainwindow::passwd.toLatin1();
    char *passwd = ba2.data();
    QByteArray ba3 = mainwindow::passwd1.toLatin1();
    char *passwdagain = ba3.data();

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
//        fprintf(stderr, ":%s\n", sqlite3_errmsg(db));
        qDebug()<<"无法打开数据库"<<sqlite3_errmsg(db);
        sqlite3_close(db);
    }
    ret = sqlite3_exec(db, sSQL2, _sql_callback, 0, &pErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL insert error: %s\n", pErrMsg);
        qDebug()<<"SQL insert error"<<pErrMsg;
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }//需要加密
    sqlite3_close(db);
    db = 0;
    ui->label->setText("口令信息存储成功!");
    ui->label->show();
    this->repaint();
    usleep(20000);
}

void mainwindow::ChangePasswd(sqlite3* db, char* appid, int fd) {	//功能函数2(已完成)
    char *pErrMsg = 0;
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
    if (ret != SQLITE_OK) {
        qDebug()<<"无法打开数据库" <<sqlite3_errmsg(db);
        sqlite3_close(db);
    }
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
//    numsuiji = (char*)malloc(sizeof(char) * 15);
    sprintf(numsuiji, "%d", rand());//获得随机数
/*    ret = sqlite3_exec(db, sSQL2, _sql_callfuc2, (void*)numsuiji, &pErrMsg);
    if (ret != SQLITE_OK) {
        qDebug()<<"SQL error:"<<pErrMsg;
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }*/

/*    int lend = strlen(num);
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
    usleep(20000);*/
}

void mainwindow::ChangePasswdold(sqlite3* db, char* appid, int fd) {
    char *pErrMsg = 0;
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
    if (ret != SQLITE_OK) {
        qDebug()<<"无法打开数据库" <<sqlite3_errmsg(db);
        sqlite3_close(db);
    }
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
    ret = sqlite3_exec(db, sSQL2, _sql_callfucold, (void*)numsuiji, &pErrMsg);
    if (ret != SQLITE_OK) {
        qDebug()<<"SQL error:"<<pErrMsg;
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }
    sqlite3_close(db);
    db = 0;
}

void mainwindow::ChangePasswdnew(sqlite3* db, char* appid, int fd) {
    char *pErrMsg = 0;
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
    if (ret != SQLITE_OK) {
        qDebug()<<"无法打开数据库" <<sqlite3_errmsg(db);
        sqlite3_close(db);
    }
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
    ret = sqlite3_exec(db, sSQL2, _sql_callfucnew, (void*)numsuiji, &pErrMsg);
    if (ret != SQLITE_OK) {
        qDebug()<<"SQL error:"<<pErrMsg;
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }
    sqlite3_close(db);
    db = 0;
}


void mainwindow::ChangePasswdend(sqlite3* db, char* appid, int fd) {
    char *pErrMsg = 0;
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
    if (ret != SQLITE_OK) {
        qDebug()<<"无法打开数据库" <<sqlite3_errmsg(db);
        sqlite3_close(db);
    }
    int lena = strlen(appid);
    int lend = strlen(numsuiji);
    char *sSQL11 = "update accounts set boolean = '1', passwd = ' ', random = '";
    char *sSQL12 = "' where appid = '";
    char *sSQL13 = "';";
    char *sSQL1 = (char*)malloc(sizeof(char) * (79 + lend + lena));
    for(int i = 0; i < 59; i++)
        sSQL1[i] = sSQL11[i];
    for(int i = 0; i < lend; i++)
        sSQL1[i + 59] = numsuiji[i];
    for(int i = 0; i < 17; i++)
        sSQL1[i + 59 + lend] = sSQL12[i];
    for(int i = 0; i < lena; i++)
        sSQL1[i + 76 + lend] = appid[i];
    for(int i = 0; i < 2; i++)
        sSQL1[i + 76 + lend + lena] = sSQL13[i];
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

void mainwindow::SynchrKey(sqlite3* db, int fd) {	//功能函数3
    usleep(20000);
    Verify(fd);
    //这里需要与PC端通信,得到一个128bit的秘钥(32位16进制数组)
    char str[33] = "";
    srand((int) time(0));
    for(int i = 0; i < 10; i++) {
        int tmp = rand() % 16;
        if(tmp < 10)
            str[i] = tmp + '0';
        else
            str[i] = tmp - 10 + 'a';
    }
    QString strall = "请在电脑端输入：";
    strall.append(str);
/*    ui->label->setText(strall);
    ui->label->show();
    this->repaint();*/
    ui->label->hide();
    this->repaint();
    QMessageBox::information(this,"提示",strall);
    for(int i = 10; i < 14; i++) {
        str[i] = '0';
    }
    FILE * fp1 = fopen("en3.txt", "w");//打开输入文件
    for(int i = 0; i < 14; i++)
        fprintf(fp1, "%c", str[i]);
    fclose(fp1);
    FILE * pp1 = popen("./en3 < en3.txt", "r"); //建立管道
    char buffe1[21];
    fgets(buffe1, sizeof(buffe1), pp1);
    char buffer1[21];
    for(int i = 0; i < 21; i++)
        buffer1[i] = buffe1[i];
    pclose(pp1);
    remove("en3.txt");

    for(int i = 0; i < 33; i++)
        statickey[i] = '\0';
    strcpy(statickey, buffer1);
    ui->label->setText("同步密钥成功!");
    ui->label->show();
    this->repaint();

    time_t t;
    statictime = time(&t);
    usleep(20000);
        //把str配置成en de的参数并编译
        //SendToPC(str);
  //  }
}

void mainwindow::Login(sqlite3* db,char* appid, int fd) {	//功能函数4(已完成)
    char *pErrMsg = 0;
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "无法打开数据库:%s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
        }
    char *sSQL3 = "select * from accounts;";

 //   typedef void* (*FUNC)(void*);//定义FUNC类型是一个指向函数的指针，该函数参数为void*，返回值为void*
 //   FUNC callback = (FUNC)&mainwindow::_sql_callsearch;//强制转换func()的类型

/*    ret = sqlite3_exec(db, sSQL3, _sql_callsearch, 0, &pErrMsg);  //这里调用了callsearch
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", pErrMsg);
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }
    scanf("%s", appid);
    getchar();*/
    time_t t;
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

void mainwindow::AccountDel(sqlite3* db) {	//功能函数5(已完成)
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

/*    char appid[20];
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
    }*/
    sqlite3_close(db);
    db = 0;
    printf("删除账号成功!\n");
    usleep(20000);
}

void Reset(sqlite3* db) {		//功能函数6(已完成)
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
/*    printf("初始化中...");
    usleep(20000);
    printf("成功\n");*/
    sqlite3_close(db);
    db = 0;
    usleep(20000);
//    remove("main.db");
}

mainwindow::mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainwindow)
{
    ui->setupUi(this);

    this->setAutoFillBackground(true);
    QPalette palette;
    QPixmap pixmap(":/new/pic1/2.jpg");
    palette.setBrush(QPalette::Background, QBrush(pixmap));
    this->setPalette(palette);
    ui->quit->hide();
    //this->setWindowFlags (Qt::CustomizeWindowHint);  Qt::WindowStaysOnTopHint
    ui->addaccount->hide();
    ui->changepasswd->hide();
    ui->deleteaccount->hide();
    ui->synchrkey->hide();
    ui->login->hide();
    ui->reset->hide();
    ui->label->hide();
    ui->title->hide();
    ui->label->setAlignment(Qt::AlignCenter);
    ui->title->setAlignment(Qt::AlignCenter);
    ui->formLayoutWidget->hide();
    ui->deletecombo->hide();
    ui->logincombo->hide();
    ui->deletelabel->hide();
    ui->delok->hide();
    ui->label1->hide();
    ui->label2->hide();
    ui->label3->hide();
    ui->label4->hide();
    ui->label5->hide();
    ui->label6->hide();
    ui->label0->setAlignment(Qt::AlignCenter);
    ui->label1->setAlignment(Qt::AlignCenter);
    ui->label2->setAlignment(Qt::AlignCenter);
    ui->label3->setAlignment(Qt::AlignCenter);
    ui->label4->setAlignment(Qt::AlignCenter);
    ui->label5->setAlignment(Qt::AlignCenter);
    ui->label6->setAlignment(Qt::AlignCenter);
    ui->loginok->hide();
    ui->changecombo->hide();
    ui->changeconfirm->hide();
    ui->changeok->hide();
    ui->inputnew->hide();
    ui->inputold->hide();
    ui->changelabel->hide();
    ui->xquit->hide();
    ui->xquit1->hide();
    ui->xquit2->hide();
    ui->changelabel->setAlignment(Qt::AlignCenter);
    ui->deletelabel->setAlignment(Qt::AlignCenter);
    this->setWindowFlags (Qt::FramelessWindowHint/*|Qt::X11BypassWindowManagerHint*/);

    QRegExp regx("[a-zA-Z0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->passwd );
    ui->appid->setValidator( validator );
    ui->id->setValidator( validator );
    ui->passwd->setValidator( validator );
    ui->passwd1->setValidator( validator );
    ui->appid->setMaxLength(20);
    ui->id->setMaxLength(20);
    ui->passwd->setMaxLength(20);
    ui->passwd1->setMaxLength(20);
    ui->jianpan->hide();
    ui->add1->hide();
    ui->add2->hide();
    ui->add3->hide();

    connect(ui->enter, SIGNAL(clicked()), this, SLOT(enter()));
    connect(ui->quit, SIGNAL(clicked()),this, SLOT(quit()));
    connect(ui->synchrkey, SIGNAL(clicked()),this, SLOT(synchrkey()));
    connect(ui->reset, SIGNAL(clicked()),this, SLOT(reset()));
    connect(ui->addaccount, SIGNAL(clicked()),this, SLOT(addaccount()));
    connect(ui->deleteaccount, SIGNAL(clicked()),this, SLOT(deleteaccount()));
    connect(ui->login, SIGNAL(clicked()),this, SLOT(login()));
    connect(ui->changepasswd, SIGNAL(clicked()),this, SLOT(changepasswd()));
    connect(ui->appid, SIGNAL(textChanged(QString)),this,SLOT(appidtextChanged(QString)));
    connect(ui->id, SIGNAL(textChanged(QString)),this,SLOT(idtextChanged(QString)));
    connect(ui->passwd, SIGNAL(textChanged(QString)),this,SLOT(passwdtextChanged(QString)));
    connect(ui->passwd1, SIGNAL(textChanged(QString)),this,SLOT(passwd1textChanged(QString)));
    connect(ui->ok, SIGNAL(clicked()),this, SLOT(addaccountok()));
    connect(ui->delok, SIGNAL(clicked()),this, SLOT(delok()));
    connect(ui->deletecombo, SIGNAL(currentIndexChanged(QString)),this,SLOT(deletetext(QString)));
    connect(ui->deletecombo, SIGNAL(activated(int)),this,SLOT(deleteint(int)));
    connect(ui->loginok, SIGNAL(clicked()),this, SLOT(loginok()));
    connect(ui->changeok, SIGNAL(clicked()),this, SLOT(changeok()));
    connect(ui->changecombo, SIGNAL(activated(QString)),this,SLOT(changetext(QString)));
    connect(ui->logincombo, SIGNAL(activated(QString)),this,SLOT(logintext(QString)));
    connect(ui->changeconfirm, SIGNAL(clicked()),this, SLOT(changeconfirm()));
    connect(ui->inputold, SIGNAL(clicked()),this, SLOT(on_inputold_clicked()));
    connect(ui->inputnew, SIGNAL(clicked()),this, SLOT(on_inputnew_clicked()));
    connect(ui->xquit, SIGNAL(clicked()),this, SLOT(on_xquit_clicked()));
    connect(ui->xquit1, SIGNAL(clicked()),this, SLOT(on_xquit1_clicked()));
    connect(ui->xquit2, SIGNAL(clicked()),this, SLOT(on_xquit2_clicked()));
    connect(ui->jianpan, SIGNAL(clicked()),this, SLOT(on_jianpan_clicked()));
    connect(ui->add1, SIGNAL(clicked()),this, SLOT(on_add1_clicked()));
    connect(ui->add2, SIGNAL(clicked()),this, SLOT(on_add2_clicked()));
    connect(ui->add3, SIGNAL(clicked()),this, SLOT(on_add3_clicked()));
    numsuiji = (char*)malloc(sizeof(char) * 15);
    ui->id->setPlaceholderText(QString::fromLocal8Bit("请输入帐号"));
    ui->appid->setPlaceholderText(QString::fromLocal8Bit("请输入AppID"));
    ui->passwd->setPlaceholderText(QString::fromLocal8Bit("请输入密码"));
    ui->passwd1->setPlaceholderText(QString::fromLocal8Bit("请输入密码"));
/*    QWidget *widget = new QWidget();
    widget->setAutoFillBackground(true);
    QPalette palette;
    QPixmap pixmap(":/new/pic1/1.png");
    palette.setBrush(QPalette::Window, QBrush(pixmap));
    widget->setPalette(palette);
    widget->show();*/
}

mainwindow::~mainwindow()
{
    delete ui;
}

void mainwindow::quit()
{
    this->close();
}

void mainwindow::enter()
{
    Initall();
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    Empty(fd);
    ui->label->setText("请按指纹");
    ui->label->show();
    this->repaint();
    EnterFingerprint(fd);//第一次录入指纹
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    this->repaint();
    StoreDB(fd, db);//把指纹消息存入数据库
    ::close(fd);
}

void mainwindow::synchrkey()
{
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    SynchrKey(db,fd);
    ::close(fd);
    usleep(500000);
    ui->label->hide();
    this->repaint();
    //printf("\n\n%d",i);
}

void mainwindow::reset()
{
    judge();
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    Empty(fd);
/*    ui->label->setText("请按指纹");
    ui->label->show();
    this->repaint();*/
  //      mb = new QMessageBox(this);
    int answer = /*mb->*/QMessageBox::question(this, "提示", "真的要恢复出厂设置吗", QMessageBox::No | QMessageBox::Yes);
    if (answer == QMessageBox::Yes){
        Reset(db);
        ui->label->setText("初始化中...");
        ui->label->show();
        this->repaint();
        sleep(1);
        ui->label->setText("成功");
        ui->label->show();
        this->repaint();
        sleep(1);
        ui->enter->show();
        ui->label0->show();
        ui->addaccount->hide();
        ui->changepasswd->hide();
        ui->deleteaccount->hide();
        ui->synchrkey->hide();
        ui->login->hide();
        ui->reset->hide();
        ui->label->hide();
        ui->deletecombo->clear();
        ui->changecombo->clear();
        ui->logincombo->clear();
        ui->label1->hide();
        ui->label2->hide();
        ui->label3->hide();
        ui->label4->hide();
        ui->label5->hide();
        ui->label6->hide();
        this->repaint();
    }
    ::close(fd);
}

void mainwindow::addaccount()
{
    judge();
    /*QFormLayout *pLayout = new QFormLayout();
    QLineEdit *pUserLineEdit = new QLineEdit();
    QLineEdit *pPasswordLineEdit = new QLineEdit();
    QLineEdit *pVerifyLineEdit = new QLineEdit();
    pLayout->addRow(QStringLiteral("用户名："), pUserLineEdit);
    pLayout->addRow(QStringLiteral("密码："), pPasswordLineEdit);
    pLayout->addRow(QStringLiteral("验证码："), pVerifyLineEdit);
    pLayout->setSpacing(10);
    pLayout->setMargin(10);
    setLayout(pLayout);*/
    ui->addaccount->hide();
    ui->changepasswd->hide();
    ui->deleteaccount->hide();
    ui->synchrkey->hide();
    ui->login->hide();
    ui->reset->hide();
    ui->label->hide();
    ui->enter->hide();
    ui->title->setText("添加帐号");
    ui->title->show();
    ui->label->hide();
    ui->label1->hide();
    ui->label2->hide();
    ui->label3->hide();
    ui->label4->hide();
    ui->label5->hide();
    ui->label6->hide();
    ui->formLayoutWidget->show();
    ui->xquit->show();
    ui->jianpan->show();
    ui->add1->show();
    ui->add2->show();
    ui->add3->show();
    this->repaint();
//    ui->formLayoutWidget->.
}

void mainwindow::on_jianpan_clicked()
{
    disconnect(ui->jianpan, SIGNAL(clicked()),this, SLOT(on_jianpan_clicked()));
    system("matchbox-keyboard -s 100 extended &");
    connect(ui->jianpan, SIGNAL(clicked()),this, SLOT(on_jianpan_clicked()));
}

void mainwindow::on_add1_clicked()
{
    ui->appid->setText("qq");
    ui->id->setText("3130486741");
    ui->passwd->setText("liangbai002");
    ui->passwd1->setText("liangbai002");
}

void mainwindow::on_add2_clicked()
{
    ui->appid->setText("163");
    ui->id->setText("bnhony");
    ui->passwd->setText("bnhonyuoyuqi");
    ui->passwd1->setText("bnhonyuoyuqi");
}

void mainwindow::on_add3_clicked()
{
    ui->appid->setText("zhongguoyinhang");
    ui->id->setText("qll19950717");
    ui->passwd->setText("qll170923");
    ui->passwd1->setText("qll170923");
}

void mainwindow::appidtextChanged(const QString appid)
{
    qDebug() << appid;
    mainwindow::appid = appid;
/*    QByteArray ba = appid.toLatin1();
    char *ch = ba.data();
    int lena = strlen(ch);
    qDebug() << lena;*/
}

void mainwindow::idtextChanged(const QString id)
{
    qDebug() << id;
    mainwindow::id = id;
}

void mainwindow::passwdtextChanged(const QString passwd)
{
    qDebug() << passwd;
    mainwindow::passwd = passwd;
}

void mainwindow::passwd1textChanged(const QString passwd1)
{
    qDebug() << passwd1;
    mainwindow::passwd1 = passwd1;
    qDebug() << mainwindow::passwd1;
}

void mainwindow::addaccountok()
{
    judge();
    if(appid ==""){
        QMessageBox::information(this,"提示","请输入AppId");
    }
    else if(id ==""){
        QMessageBox::information(this,"提示","请输入用户名");
    }
    else if(passwd ==""){
        QMessageBox::information(this,"提示","请输入口令");
    }
    else if(passwd1 ==""){
        QMessageBox::information(this,"提示","请再次输入口令");
    }
    else if(passwd != passwd1){
        QMessageBox::information(this,"提示","两次输入的口令要相同");
    }
    else{
        int answer = /*mb->*/QMessageBox::question(this, "提示", "是否修改为复杂口令？", QMessageBox::No | QMessageBox::Yes);
        if (answer == QMessageBox::No){
            ui->jianpan->hide();
            ui->add1->hide();
            ui->add2->hide();
            ui->add3->hide();
            ui->enter->hide();
            ui->xquit->hide();
            ui->addaccount->show();
            ui->changepasswd->show();
            ui->deleteaccount->show();
            ui->synchrkey->show();
            ui->login->show();
            ui->reset->show();
            ui->label->hide();
            ui->title->hide();
            ui->formLayoutWidget->hide();
            ui->label0->hide();
            ui->label1->show();
            ui->label2->show();
            ui->label3->show();
            ui->label4->show();
            ui->label5->show();
            ui->label6->show();
            this->repaint();
            int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
            AddAccount(db,fd);
            ::close(fd);
            ui->deletecombo->addItem(appid);
            ui->logincombo->addItem(appid);
            ui->changecombo->addItem(appid);
            ui->appid->clear();
            ui->id->clear();
            ui->passwd->clear();
            ui->passwd1->clear();
            usleep(200000);
            ui->label->hide();
            this->repaint();
        }
        else{
            int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
            AddAccount(db,fd);
            ::close(fd);
            ui->jianpan->hide();
            ui->add1->hide();
            ui->add2->hide();
            ui->add3->hide();
            ui->deletecombo->addItem(appid);
            ui->logincombo->addItem(appid);
            ui->changecombo->addItem(appid);
            ui->appid->clear();
            ui->id->clear();
            ui->passwd->clear();
            ui->passwd1->clear();
            ui->label->hide();
            ui->title->hide();
            ui->formLayoutWidget->hide();
            ui->xquit->hide();
            ui->addaccount->hide();
            ui->changepasswd->hide();
            ui->deleteaccount->hide();
            ui->synchrkey->hide();
            ui->login->hide();
            ui->reset->hide();
            ui->label->hide();
            ui->enter->hide();
            ui->title->setText("修改口令");
            ui->title->show();
        //    ui->delok->show();
            ui->label->hide();
            ui->label1->hide();
            ui->label2->hide();
            ui->label3->hide();
            ui->label4->hide();
            ui->label5->hide();
            ui->label6->hide();
            ui->changecombo->show();
            ui->deletelabel->setText("请选择需要修改的帐号：");
            ui->deletelabel->show();
            ui->changeconfirm->show();
            ui->changeok->show();

            this->repaint();
            changepasswd();
        }
    }
}

void mainwindow::deleteaccount()
{
    judge();
    AccountDel(db);
    ui->addaccount->hide();
    ui->changepasswd->hide();
    ui->deleteaccount->hide();
    ui->synchrkey->hide();
    ui->login->hide();
    ui->reset->hide();
    ui->label->hide();
    ui->enter->hide();
    ui->title->setText("删除帐号");
    ui->title->show();
    ui->delok->show();
    ui->label->hide();
    ui->label1->hide();
    ui->label2->hide();
    ui->label3->hide();
    ui->label4->hide();
    ui->label5->hide();
    ui->label6->hide();
    ui->xquit2->show();
//    ui->deletecombo->addItem("xx");
//    QString str = QString(QLatin1String("xx"));
//    ui->deletecombo->addItem(str);

/*    for(int i = 0;i <= deli; i++){
     //   ui->deletecombo->addItem("xx");
        QString str = QString(QLatin1String(delarry[i]));
        ui->deletecombo->addItem(str);
    }*/

    ui->deletecombo->show();
    ui->deletelabel->setText("请选择需要删除的帐号：");
    ui->deletelabel->show();
    this->repaint();
//    ui->deletecombo->clear();
    //ui->deletecombo->addItems();
//    ui->formLayoutWidget->
}

void mainwindow::deletetext(const QString deltext)
{
    qDebug() << deltext;
    mainwindow::deltext = deltext;
}

void mainwindow::deleteint(const int delint)
{
    qDebug() << delint;
    mainwindow::delint = delint;
}

void mainwindow::delok()
{
    judge();
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->delok->hide();
    ui->deletecombo->hide();
    ui->deletelabel->hide();
    ui->formLayoutWidget->hide();
    ui->xquit2->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
/*    for(int i = ui->deletecombo->count()-1;i>=0;i--){
        ui->deletecombo->rmoveItem(i);
        qDebug() << i;
    }*/
    ui->deletecombo->removeItem(delint);
    ui->logincombo->removeItem(delint);
    ui->changecombo->removeItem(delint);

    QByteArray ba = deltext.toLatin1();
    int ret = 0;
    ret = sqlite3_open("./main.db", &db);
    char *appid = ba.data();
//    char appid[20];
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
    char *pErrMsg = 0;
    ret = sqlite3_exec(db, sSQL2, NULL, 0, &pErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", pErrMsg);
        sqlite3_free(pErrMsg);
        sqlite3_close(db);
    }
    sqlite3_close(db);
    db = 0;
//    sleep(1);
    ui->label->setText("删除账号成功!");
    ui->label->show();
    this->repaint();
    usleep(500000);
    ui->label->hide();
    this->repaint();
}

void mainwindow::login()
{
    judge();
    time_t t;
    int now = time(&t);
    if (now - statictime > 180){
        QMessageBox::information(this,"提示","密钥已经过期,请先同步密钥");
        synchrkey();
    }
    else{
        ui->addaccount->hide();
        ui->changepasswd->hide();
        ui->deleteaccount->hide();
        ui->synchrkey->hide();
        ui->login->hide();
        ui->reset->hide();
        ui->label->hide();
        ui->enter->hide();
        ui->title->setText("登录帐号");
        ui->title->show();
        ui->loginok->show();
        ui->label->hide();
        ui->label1->hide();
        ui->label2->hide();
        ui->label3->hide();
        ui->label4->hide();
        ui->label5->hide();
        ui->label6->hide();
        ui->xquit1->show();
        ui->logincombo->show();
        ui->deletelabel->setText("请选择需要登录的帐号：");
        ui->deletelabel->show();
        this->repaint();
    }
}

void mainwindow::logintext(const QString logtext)
{
    mainwindow::logtext = logtext;
    qDebug()<< mainwindow::logtext;
}

void mainwindow::loginok()
{
    judge();
    ui->label->setText("正在发送...");
    ui->label->show();
    this->repaint();
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    QByteArray ba = logtext.toLatin1();
    char *ch = ba.data();
    Login(db, ch, fd);
    ::close(fd);
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->loginok->hide();
    ui->logincombo->hide();
    ui->deletelabel->hide();
    ui->formLayoutWidget->hide();
    ui->xquit1->hide();
    ui->label->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    this->repaint();
}

void mainwindow::changepasswd()
{
    judge();
    time_t t;
    int now = time(&t);
    if (now - statictime > 180){
        QMessageBox::information(this,"提示","秘钥已经过期,请先同步密钥");
        synchrkey();
    }
    else{
        ui->addaccount->hide();
        ui->changepasswd->hide();
        ui->deleteaccount->hide();
        ui->synchrkey->hide();
        ui->login->hide();
        ui->reset->hide();
        ui->label->hide();
        ui->enter->hide();
        ui->title->setText("修改口令");
        ui->title->show();
    //    ui->delok->show();
        ui->label->hide();
        ui->label1->hide();
        ui->label2->hide();
        ui->label3->hide();
        ui->label4->hide();
        ui->label5->hide();
        ui->label6->hide();
        ui->changecombo->show();
        ui->deletelabel->setText("请选择需要修改的帐号：");
        ui->deletelabel->show();
        ui->changeconfirm->show();
        ui->changeok->show();
        this->repaint();
    }
}

void mainwindow::changeok()
{
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->changecombo->hide();
    ui->changeconfirm->hide();
    ui->changeok->hide();
    ui->inputnew->hide();
    ui->inputold->hide();
    ui->deletelabel->hide();
    ui->formLayoutWidget->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    ui->changelabel->hide();
    this->repaint();
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    QByteArray ba = chatext.toLatin1();
    char *ch = ba.data();
    ChangePasswdend(db, ch, fd);
    ::close(fd);
}

void mainwindow::changetext(const QString chatext)
{
    mainwindow::chatext = chatext;
}

void mainwindow::changeconfirm()
{
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    ui->changelabel->show();
    ui->inputnew->show();
    ui->inputold->show();
    this->repaint();
    qDebug() <<mainwindow::chatext;
    QByteArray ba = chatext.toLatin1();
    char *ch = ba.data();
    qDebug()<<ch;
    ChangePasswd(db, ch, fd);
    ::close(fd);
}

void mainwindow::on_inputold_clicked()
{
    disconnect(ui->inputold, SIGNAL(clicked()),this, SLOT(on_inputold_clicked()));
    ui->label->setText("正在发送...");
    ui->label->show();
    this->repaint();
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    QByteArray ba = chatext.toLatin1();
    char *ch = ba.data();
    ChangePasswdold(db, ch, fd);
    qDebug()<<"old fawanle!";
    ::close(fd);
    ui->label->hide();
    this->repaint();
    connect(ui->inputold, SIGNAL(clicked()),this, SLOT(on_inputold_clicked()));
}

void mainwindow::on_inputnew_clicked()
{
    disconnect(ui->inputnew, SIGNAL(clicked()),this, SLOT(on_inputnew_clicked()));
    ui->label->setText("正在发送...");
    ui->label->show();
    this->repaint();
    int fd = open("/dev/ttyUSB1", O_RDWR|O_NOCTTY|O_NDELAY);
    QByteArray ba = chatext.toLatin1();
    char *ch = ba.data();
    ChangePasswdnew(db, ch, fd);
    qDebug()<<"new fawanle!";
    ::close(fd);
    ui->label->hide();
    this->repaint();
    connect(ui->inputnew, SIGNAL(clicked()),this, SLOT(on_inputnew_clicked()));
}

void mainwindow::on_xquit_clicked()
{
    ui->enter->hide();
    ui->xquit->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->formLayoutWidget->hide();
    ui->add1->hide();
    ui->add2->hide();
    ui->add3->hide();
    ui->jianpan->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    this->repaint();
    ui->appid->clear();
    ui->id->clear();
    ui->passwd->clear();
    ui->passwd1->clear();
}

void mainwindow::on_xquit1_clicked()
{
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->loginok->hide();
    ui->logincombo->hide();
    ui->deletelabel->hide();
    ui->formLayoutWidget->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    ui->xquit1->hide();
    this->repaint();
}

void mainwindow::on_xquit2_clicked()
{
    ui->enter->hide();
    ui->addaccount->show();
    ui->changepasswd->show();
    ui->deleteaccount->show();
    ui->synchrkey->show();
    ui->login->show();
    ui->reset->show();
    ui->label->hide();
    ui->title->hide();
    ui->delok->hide();
    ui->deletecombo->hide();
    ui->deletelabel->hide();
    ui->formLayoutWidget->hide();
    ui->label0->hide();
    ui->label1->show();
    ui->label2->show();
    ui->label3->show();
    ui->label4->show();
    ui->label5->show();
    ui->label6->show();
    ui->xquit2->hide();
    this->repaint();
}

