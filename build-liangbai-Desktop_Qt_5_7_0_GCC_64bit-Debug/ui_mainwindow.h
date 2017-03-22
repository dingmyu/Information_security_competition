/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mainwindow
{
public:
    QPushButton *enter;
    QPushButton *quit;
    QPushButton *addaccount;
    QPushButton *login;
    QPushButton *changepasswd;
    QPushButton *deleteaccount;
    QPushButton *synchrkey;
    QPushButton *reset;
    QLabel *label;
    QLabel *title;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label_2;
    QLineEdit *appid;
    QLabel *label_3;
    QLineEdit *id;
    QLabel *label_4;
    QLineEdit *passwd;
    QLabel *label_5;
    QLineEdit *passwd1;
    QPushButton *ok;
    QLabel *deletelabel;
    QComboBox *deletecombo;
    QPushButton *delok;
    QLabel *label1;
    QLabel *label2;
    QLabel *label3;
    QLabel *label4;
    QLabel *label5;
    QLabel *label6;
    QLabel *label0;
    QComboBox *logincombo;
    QPushButton *loginok;
    QComboBox *changecombo;
    QPushButton *changeok;
    QPushButton *changeconfirm;
    QPushButton *inputold;
    QPushButton *inputnew;
    QLabel *changelabel;
    QPushButton *xquit;
    QPushButton *xquit1;
    QPushButton *xquit2;
    QPushButton *jianpan;
    QPushButton *add1;
    QPushButton *add2;
    QPushButton *add3;

    void setupUi(QWidget *mainwindow)
    {
        if (mainwindow->objectName().isEmpty())
            mainwindow->setObjectName(QStringLiteral("mainwindow"));
        mainwindow->resize(480, 320);
        mainwindow->setStyleSheet(QStringLiteral(""));
        enter = new QPushButton(mainwindow);
        enter->setObjectName(QStringLiteral("enter"));
        enter->setGeometry(QRect(210, 130, 71, 61));
        enter->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\346\267\273\345\212\240\347\224\250\346\210\267.png);"));
        quit = new QPushButton(mainwindow);
        quit->setObjectName(QStringLiteral("quit"));
        quit->setGeometry(QRect(390, 290, 80, 23));
        addaccount = new QPushButton(mainwindow);
        addaccount->setObjectName(QStringLiteral("addaccount"));
        addaccount->setGeometry(QRect(70, 40, 80, 81));
        addaccount->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\346\267\273\345\212\240\350\264\246\345\217\267.png);"));
        login = new QPushButton(mainwindow);
        login->setObjectName(QStringLiteral("login"));
        login->setGeometry(QRect(200, 40, 80, 81));
        login->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\347\231\273\345\275\2252.png);"));
        changepasswd = new QPushButton(mainwindow);
        changepasswd->setObjectName(QStringLiteral("changepasswd"));
        changepasswd->setGeometry(QRect(330, 40, 80, 81));
        changepasswd->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\344\277\256\346\224\271\345\217\243\344\273\244.png);"));
        deleteaccount = new QPushButton(mainwindow);
        deleteaccount->setObjectName(QStringLiteral("deleteaccount"));
        deleteaccount->setGeometry(QRect(70, 180, 80, 81));
        deleteaccount->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\345\210\240\351\231\244\350\264\246\345\217\267.png);"));
        synchrkey = new QPushButton(mainwindow);
        synchrkey->setObjectName(QStringLiteral("synchrkey"));
        synchrkey->setGeometry(QRect(200, 180, 80, 81));
        synchrkey->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\345\220\214\346\255\245\345\257\206\351\222\245.png);"));
        reset = new QPushButton(mainwindow);
        reset->setObjectName(QStringLiteral("reset"));
        reset->setGeometry(QRect(330, 180, 80, 81));
        reset->setStyleSheet(QString::fromUtf8("border-image: url(:/new/pic1/\345\233\276\346\240\207/\346\201\242\345\244\215\345\233\276\346\240\207.png);"));
        label = new QLabel(mainwindow);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(0, 290, 481, 20));
        label->setLayoutDirection(Qt::LeftToRight);
        title = new QLabel(mainwindow);
        title->setObjectName(QStringLiteral("title"));
        title->setGeometry(QRect(200, 20, 81, 20));
        formLayoutWidget = new QWidget(mainwindow);
        formLayoutWidget->setObjectName(QStringLiteral("formLayoutWidget"));
        formLayoutWidget->setGeometry(QRect(120, 90, 241, 180));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(formLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        appid = new QLineEdit(formLayoutWidget);
        appid->setObjectName(QStringLiteral("appid"));
        appid->setMinimumSize(QSize(114, 0));

        formLayout->setWidget(1, QFormLayout::FieldRole, appid);

        label_3 = new QLabel(formLayoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        id = new QLineEdit(formLayoutWidget);
        id->setObjectName(QStringLiteral("id"));

        formLayout->setWidget(2, QFormLayout::FieldRole, id);

        label_4 = new QLabel(formLayoutWidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        passwd = new QLineEdit(formLayoutWidget);
        passwd->setObjectName(QStringLiteral("passwd"));

        formLayout->setWidget(3, QFormLayout::FieldRole, passwd);

        label_5 = new QLabel(formLayoutWidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_5);

        passwd1 = new QLineEdit(formLayoutWidget);
        passwd1->setObjectName(QStringLiteral("passwd1"));

        formLayout->setWidget(4, QFormLayout::FieldRole, passwd1);

        ok = new QPushButton(formLayoutWidget);
        ok->setObjectName(QStringLiteral("ok"));

        formLayout->setWidget(5, QFormLayout::SpanningRole, ok);

        deletelabel = new QLabel(mainwindow);
        deletelabel->setObjectName(QStringLiteral("deletelabel"));
        deletelabel->setGeometry(QRect(170, 110, 141, 16));
        deletecombo = new QComboBox(mainwindow);
        deletecombo->setObjectName(QStringLiteral("deletecombo"));
        deletecombo->setGeometry(QRect(200, 150, 79, 23));
        delok = new QPushButton(mainwindow);
        delok->setObjectName(QStringLiteral("delok"));
        delok->setGeometry(QRect(200, 250, 80, 23));
        label1 = new QLabel(mainwindow);
        label1->setObjectName(QStringLiteral("label1"));
        label1->setGeometry(QRect(70, 130, 81, 16));
        label2 = new QLabel(mainwindow);
        label2->setObjectName(QStringLiteral("label2"));
        label2->setGeometry(QRect(200, 130, 81, 16));
        label3 = new QLabel(mainwindow);
        label3->setObjectName(QStringLiteral("label3"));
        label3->setGeometry(QRect(320, 130, 101, 16));
        label4 = new QLabel(mainwindow);
        label4->setObjectName(QStringLiteral("label4"));
        label4->setGeometry(QRect(70, 270, 81, 16));
        label5 = new QLabel(mainwindow);
        label5->setObjectName(QStringLiteral("label5"));
        label5->setGeometry(QRect(200, 270, 81, 16));
        label6 = new QLabel(mainwindow);
        label6->setObjectName(QStringLiteral("label6"));
        label6->setGeometry(QRect(330, 270, 81, 16));
        label0 = new QLabel(mainwindow);
        label0->setObjectName(QStringLiteral("label0"));
        label0->setGeometry(QRect(210, 200, 59, 15));
        logincombo = new QComboBox(mainwindow);
        logincombo->setObjectName(QStringLiteral("logincombo"));
        logincombo->setGeometry(QRect(200, 150, 79, 23));
        loginok = new QPushButton(mainwindow);
        loginok->setObjectName(QStringLiteral("loginok"));
        loginok->setGeometry(QRect(200, 250, 80, 23));
        changecombo = new QComboBox(mainwindow);
        changecombo->setObjectName(QStringLiteral("changecombo"));
        changecombo->setGeometry(QRect(200, 150, 79, 23));
        changeok = new QPushButton(mainwindow);
        changeok->setObjectName(QStringLiteral("changeok"));
        changeok->setGeometry(QRect(200, 250, 80, 23));
        changeconfirm = new QPushButton(mainwindow);
        changeconfirm->setObjectName(QStringLiteral("changeconfirm"));
        changeconfirm->setGeometry(QRect(290, 150, 41, 23));
        inputold = new QPushButton(mainwindow);
        inputold->setObjectName(QStringLiteral("inputold"));
        inputold->setGeometry(QRect(70, 210, 80, 23));
        inputnew = new QPushButton(mainwindow);
        inputnew->setObjectName(QStringLiteral("inputnew"));
        inputnew->setGeometry(QRect(330, 210, 80, 23));
        changelabel = new QLabel(mainwindow);
        changelabel->setObjectName(QStringLiteral("changelabel"));
        changelabel->setGeometry(QRect(160, 180, 161, 20));
        xquit = new QPushButton(mainwindow);
        xquit->setObjectName(QStringLiteral("xquit"));
        xquit->setGeometry(QRect(450, 10, 16, 16));
        xquit1 = new QPushButton(mainwindow);
        xquit1->setObjectName(QStringLiteral("xquit1"));
        xquit1->setGeometry(QRect(450, 10, 16, 16));
        xquit2 = new QPushButton(mainwindow);
        xquit2->setObjectName(QStringLiteral("xquit2"));
        xquit2->setGeometry(QRect(450, 10, 16, 16));
        jianpan = new QPushButton(mainwindow);
        jianpan->setObjectName(QStringLiteral("jianpan"));
        jianpan->setGeometry(QRect(20, 250, 21, 23));
        add1 = new QPushButton(mainwindow);
        add1->setObjectName(QStringLiteral("add1"));
        add1->setGeometry(QRect(430, 80, 21, 23));
        add2 = new QPushButton(mainwindow);
        add2->setObjectName(QStringLiteral("add2"));
        add2->setGeometry(QRect(430, 120, 21, 23));
        add3 = new QPushButton(mainwindow);
        add3->setObjectName(QStringLiteral("add3"));
        add3->setGeometry(QRect(430, 160, 21, 23));

        retranslateUi(mainwindow);

        QMetaObject::connectSlotsByName(mainwindow);
    } // setupUi

    void retranslateUi(QWidget *mainwindow)
    {
        mainwindow->setWindowTitle(QApplication::translate("mainwindow", "Palm-Passwd", 0));
        enter->setText(QString());
        quit->setText(QApplication::translate("mainwindow", "quit", 0));
        addaccount->setText(QString());
        login->setText(QString());
        changepasswd->setText(QString());
        deleteaccount->setText(QString());
        synchrkey->setText(QString());
        reset->setText(QString());
        label->setText(QApplication::translate("mainwindow", "TextLabel", 0));
        title->setText(QApplication::translate("mainwindow", "title", 0));
        label_2->setText(QApplication::translate("mainwindow", "AppID:", 0));
        label_3->setText(QApplication::translate("mainwindow", "\347\224\250\346\210\267\345\220\215\357\274\232", 0));
        label_4->setText(QApplication::translate("mainwindow", "\345\217\243\344\273\244\357\274\232", 0));
        label_5->setText(QApplication::translate("mainwindow", "\345\217\243\344\273\244again\357\274\232", 0));
        ok->setText(QApplication::translate("mainwindow", "\347\241\256\350\256\244", 0));
        deletelabel->setText(QApplication::translate("mainwindow", "\350\257\267\351\200\211\346\213\251\351\234\200\350\246\201\345\210\240\351\231\244\347\232\204\345\270\220\345\217\267\357\274\232", 0));
        delok->setText(QApplication::translate("mainwindow", "\347\241\256\350\256\244", 0));
        label1->setText(QApplication::translate("mainwindow", "\346\267\273\345\212\240\345\270\220\345\217\267", 0));
        label2->setText(QApplication::translate("mainwindow", "\347\231\273\345\275\225\345\270\220\345\217\267", 0));
        label3->setText(QApplication::translate("mainwindow", "\344\277\256\346\224\271\345\217\243\344\273\244", 0));
        label4->setText(QApplication::translate("mainwindow", "\345\210\240\351\231\244\345\270\220\345\217\267", 0));
        label5->setText(QApplication::translate("mainwindow", "\345\220\214\346\255\245\345\257\206\351\222\245", 0));
        label6->setText(QApplication::translate("mainwindow", "\346\201\242\345\244\215\345\207\272\345\216\202\350\256\276\347\275\256", 0));
        label0->setText(QApplication::translate("mainwindow", "\346\267\273\345\212\240\347\224\250\346\210\267", 0));
        loginok->setText(QApplication::translate("mainwindow", "\347\241\256\350\256\244", 0));
        changeok->setText(QApplication::translate("mainwindow", "\347\273\223\346\235\237", 0));
        changeconfirm->setText(QApplication::translate("mainwindow", "\347\241\256\350\256\244", 0));
        inputold->setText(QApplication::translate("mainwindow", "\350\276\223\345\205\245\346\227\247\345\217\243\344\273\244", 0));
        inputnew->setText(QApplication::translate("mainwindow", "\350\276\223\345\205\245\346\226\260\345\217\243\344\273\244", 0));
        changelabel->setText(QApplication::translate("mainwindow", "\345\267\262\344\270\272\346\202\250\347\224\237\346\210\220\346\226\260\347\232\204\345\217\243\344\273\244", 0));
        xquit->setText(QApplication::translate("mainwindow", "X", 0));
        xquit1->setText(QApplication::translate("mainwindow", "X", 0));
        xquit2->setText(QApplication::translate("mainwindow", "X", 0));
        jianpan->setText(QString());
        add1->setText(QApplication::translate("mainwindow", "1", 0));
        add2->setText(QApplication::translate("mainwindow", "2", 0));
        add3->setText(QApplication::translate("mainwindow", "3", 0));
    } // retranslateUi

};

namespace Ui {
    class mainwindow: public Ui_mainwindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
