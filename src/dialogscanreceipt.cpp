#include "dialogscanreceipt.h"
#include "ui_dialogscanreceipt.h"
#include <QDebug>

void DialogScanReceipt::fillCamerasList()
{
    int prev = ui->comboBox->currentIndex();
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    ui->comboBox->clear();
    for (int i = 0; i<cameras.count(); ++i)
        ui->comboBox->addItem(cameras[i].description());
    if (prev<0 || prev>=ui->comboBox->count())
        prev = 0;
    ui->comboBox->setCurrentIndex(prev);
    //on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
}

DialogScanReceipt::DialogScanReceipt(QWidget *parent) :
    QDialog(parent),
    m_Camera(nullptr),
    m_CaptureImage(nullptr),
    ui(new Ui::DialogScanReceipt)
{
    ui->setupUi(this);

    m_Decoder.setDecoder( QZXing::DecoderFormat_QR_CODE );

    fillCamerasList();
}

DialogScanReceipt::~DialogScanReceipt()
{
    delete m_Camera;
    delete ui;
}

int DialogScanReceipt::exec()
{
    if (m_CaptureImage)
        m_CaptureImage->capture("img.jpg");
    return QDialog::exec();
}

QString DialogScanReceipt::getFN()
{
    return ui->lineEdit_fn->text();
}

QString DialogScanReceipt::getFD()
{
    return ui->lineEdit_fd->text();
}

QString DialogScanReceipt::getFPD()
{
    return ui->lineEdit_fpd->text();
}

void DialogScanReceipt::on_comboBox_currentIndexChanged(int index)
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (index<0 || index>=cameras.count())
        return;
    delete m_CaptureImage;
    delete m_Camera;


    m_Camera = new QCamera(cameras[index]);


    m_CaptureImage = new QCameraImageCapture(m_Camera);
    m_CaptureImage->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    connect(m_CaptureImage,SIGNAL(imageAvailable(int,QVideoFrame)),this,SLOT(on_imageAvailable(int,QVideoFrame)));
    m_Camera->setCaptureMode(QCamera::CaptureStillImage);
    m_Camera->start();
    m_Camera->searchAndLock();

    if (m_CaptureImage->isReadyForCapture()){
        int id = m_CaptureImage->capture("img.jpg");
        qDebug() << id;
    }
    else{
        connect(m_CaptureImage,SIGNAL(readyForCaptureChanged(bool)),this,SLOT(on_readyForCaptureChanged(bool)));
    }
}

void DialogScanReceipt::on_imageAvailable(int , const QVideoFrame &buffer)
{

    if (!isVisible())
        return;
    QImage img;
    QVideoFrame frame(buffer);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(
                frame.pixelFormat());
    // BUT the frame.pixelFormat() is QVideoFrame::Format_Jpeg, and this is
    // mapped to QImage::Format_Invalid by
    // QVideoFrame::imageFormatFromPixelFormat
    if (imageFormat != QImage::Format_Invalid) {
        img = QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     // frame.bytesPerLine(),
                     imageFormat);
    } else {
        // e.g. JPEG
        int nbytes = frame.mappedBytes();
        img = QImage::fromData(frame.bits(), nbytes);
    }
    frame.unmap();

    QString data = m_Decoder.decodeImage(img);
    if (!data.isEmpty()){
        QUrlQuery query(data);
        QString fn = query.queryItemValue("fn");
        QString fd = query.queryItemValue("i");
        QString fpd = query.queryItemValue("fp");
        ui->lineEdit_fn->setText(fn);
        ui->lineEdit_fd->setText(fd);
        ui->lineEdit_fpd->setText(fpd);
        on_pushButtonManualInput_clicked();
    }

    ui->widget->setImage(img);

    m_CaptureImage->capture("img.jpg");
}

void DialogScanReceipt::on_readyForCaptureChanged(bool)
{
    m_CaptureImage->capture("img.jpg");
}

void DialogScanReceipt::on_pushButtonManualInput_clicked()
{
    if (ui->lineEdit_fn->text().isEmpty())
        return;
    if (ui->lineEdit_fd->text().isEmpty())
        return;
    if (ui->lineEdit_fpd->text().isEmpty())
        return;
    accept();
}
