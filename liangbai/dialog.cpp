#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(quit()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::quit()
{
    this->close();
    delete ui;
}

Ui::Dialog * Dialog::getUi() {
    return ui;
}


