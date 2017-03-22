#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <unistd.h>
#include <QWidget>
#include <sqlite3.h>

namespace Ui {
class mainwindow;
}

class mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = 0);
    ~mainwindow();

    void UART0_Open_Set();
    void UART1_Open_Set();
    void Initall();
    int EnterFingerprint(int fd);
    int Match(int fd);
    int Verify(int fd);
    void SynchrKey(sqlite3* db, int fd);
    void find(sqlite3* db, int fd);
    void ChangePasswd(sqlite3* db, char* appid, int fd);
    void ChangePasswdend(sqlite3* db, char* appid, int fd);
    void ChangePasswdold(sqlite3* db, char* appid, int fd);
    void ChangePasswdnew(sqlite3* db, char* appid, int fd);
    void Login(sqlite3* db, char* ippid, int fd);
    void AddAccount(sqlite3* db, int fd);
    void AccountDel(sqlite3* db);
    void insert(char *a);
    void judge();
//    int _sql_callsearch(void *notused, int argc, char **argv, char **szColName);

public slots:
    void quit();
    void enter();
    void synchrkey();
    void reset();
    void addaccount();
    void addaccountok();
    void deleteaccount();
    void delok();
    void login();
    void changepasswd();
    void loginok();
    void changeok();
    void changeconfirm();

private slots:
    void appidtextChanged(const QString appid);
    void idtextChanged(const QString id);
    void passwdtextChanged(const QString passwd);
    void passwd1textChanged(const QString passwd1);
    void deletetext(const QString deltext);
    void deleteint(const int delint);
    void changetext(const QString chatext);
    void logintext(const QString logtext);
    void on_inputold_clicked();
    void on_inputnew_clicked();
    void on_xquit_clicked();
    void on_xquit1_clicked();
    void on_xquit2_clicked();
    void on_jianpan_clicked();
    void on_add1_clicked();
    void on_add2_clicked();
    void on_add3_clicked();

private:
    Ui::mainwindow *ui;
    QString appid;
    QString id;
    QString passwd;
    QString passwd1;
    QString deltext;
    QString chatext;
    QString logtext;
    int delint;
    char *numsuiji;
 //   QMessageBox *mb;
//friend int _sql_callsearch(void *notused, int argc, char **argv, char **szColName);
};

#endif // MAINWINDOW_H
