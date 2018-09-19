#ifndef DIALOGSCANRECEIPT_H
#define DIALOGSCANRECEIPT_H

#include <QDialog>
#include <QCameraInfo>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QZXing.h>

namespace Ui {
class DialogScanReceipt;
}

class DialogScanReceipt : public QDialog
{
    Q_OBJECT
private:
    QZXing              m_Decoder;

    QCamera*            m_Camera;
    QCameraImageCapture*m_CaptureImage;
    void fillCamerasList();
public:
    explicit DialogScanReceipt(QWidget *parent = 0);
    ~DialogScanReceipt();

    virtual int exec() override;

    QString getFN();
    QString getFD();
    QString getFPD();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_imageAvailable(int, const QVideoFrame &buffer);

    void on_readyForCaptureChanged(bool);

    void on_pushButtonManualInput_clicked();

private:
    Ui::DialogScanReceipt *ui;
};

#endif // DIALOGSCANRECEIPT_H
