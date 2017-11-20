#include "surface.h"
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
Surface::Surface(QWidget *parent) : QWidget(parent)
{
reader = new Reader("rtmp://127.0.0.1/test/123");
}
void Surface::mousePressEvent(QMouseEvent *ev)
{

    if(ev->button() == Qt::RightButton)
    {
        QString filename = QFileDialog::getOpenFileName();
        if(filename.length() == 0)
            return;

        if(reader != NULL)
        {
            reader->quit();
            delete reader;
        }

        reader = new Reader(filename);
        connect(reader, SIGNAL(sigNewImage(QImage)),
                this, SLOT(slotNewImage(QImage)));
    }
}

void Surface::paintEvent(QPaintEvent *)
{
    if(image.isNull()) return;

    QPainter p(this);
    p.drawImage(0, 0,
                image.scaled(size(), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation));
}

void Surface::slotNewImage(QImage image)
{
    this->image = image;
    update();
}
