#ifndef CASTER_H
#define CASTER_H

#include <QWidget>
#include<QApplication>
#include<QSystemTrayIcon>
#include<QProcess>

namespace Ui {
class Caster;
}

class Caster : public QWidget
{
    Q_OBJECT

public:
    explicit Caster(QWidget *parent = 0);
    QSystemTrayIcon* icon;
    QProcess ffmpeg;

    bool bStart;
    void start();
    void stop();

    ~Caster();
public slots:
    void slotStartOrStop();

private:
    Ui::Caster *ui;
};

#endif // CASTER_H
