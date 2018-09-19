#include "dialogcopytext.h"
#include "ui_dialogcopytext.h"
#include <QApplication>
#include <QClipboard>

DialogCopyText::DialogCopyText(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCopyText)
{
    ui->setupUi(this);
}

DialogCopyText::~DialogCopyText()
{
    delete ui;
}

void DialogCopyText::exec(QStringList text)
{
    QString textToCopy = text.join("\n");
    ui->plainTextEdit->setPlainText(textToCopy);

    QApplication::clipboard()->setText(textToCopy);

    QDialog::exec();
}

void DialogCopyText::on_pushButton_clicked()
{
    QApplication::clipboard()->setText(ui->plainTextEdit->toPlainText());
}
