#ifndef CWIMAGE_H
#define CWIMAGE_H

#include <QWidget>
#include <QImage>

class cwImage: public QWidget
{
    Q_OBJECT

protected:
    QImage              m_Image;
public:
    cwImage(QWidget *parent);

    virtual void paintEvent(QPaintEvent *) override;
    void setImage(QImage& image){
        m_Image = image;
        update();
    }
};

#endif // CWIMAGE_H
