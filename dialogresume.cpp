#include "dialogresume.h"
#include "ui_dialogresume.h"

DialogResume::DialogResume(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogResume)
{
    ui->setupUi(this);
    ui->textBrowser_resume->setLineWrapMode(QTextEdit::WidgetWidth);
}

DialogResume::~DialogResume()
{
    delete ui;
}
void DialogResume::setResumeText(const QString &texte)
{
    ui->textBrowser_resume->setPlainText(texte);
}
