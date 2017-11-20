#ifndef SURFACE_H
#define SURFACE_H

#include <QWidget>
#include "reader.h"
class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
        QImage image;
        Reader* reader;
signals:

public slots:
        void slotNewImage(QImage image);
};

#endif // SURFACE_H
