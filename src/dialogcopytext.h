#ifndef DIALOGCOPYTEXT_H
#define DIALOGCOPYTEXT_H

#include <QDialog>

namespace Ui {
class DialogCopyText;
}

class DialogCopyText : public QDialog
{
    Q_OBJECT
public:
    explicit DialogCopyText(QWidget *parent = 0);
    ~DialogCopyText();

    void exec(QStringList text);

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogCopyText *ui;
};

#endif // DIALOGCOPYTEXT_H
