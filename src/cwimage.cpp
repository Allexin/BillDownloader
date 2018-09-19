#include "cwimage.h"
#include <QPainter>

cwImage::cwImage(QWidget* parent):QWidget(parent)
{

}

void cwImage::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawImage(QRect(0,0,width(),height()),m_Image.scaled(width(),height(),Qt::KeepAspectRatio));
}
