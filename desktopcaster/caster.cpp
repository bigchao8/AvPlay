#include "caster.h"
#include "ui_caster.h"
#include <QIcon>
#include <QMenu>
#include <unistd.h>
#include <QDebug>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
Caster::Caster(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Caster)
{
    ui->setupUi(this);
    bStart=false;
    QMenu* menu=new QMenu;

    menu->addAction("start upstream",this,SLOT(slotStartOrStop()));
    menu->addAction("exit",qApp,SLOT(quit()));

    icon=new QSystemTrayIcon(this);
    icon->setIcon(QIcon(":/stop.png"));
    icon->setContextMenu(menu);
    icon->show();
}
void Caster::slotStartOrStop()
{
    QAction* action=(QAction*)sender();
    if(this->bStart==false)
    {
        qDebug()<<"start";
        start();
        this->bStart=true;
        action->setText("stop upstream");
    }else{
        qDebug()<<"stop";
        stop();
        this->bStart=false;
        action->setText("start up");
    }
}
void Caster::stop()
{
ffmpeg.kill();
ffmpeg.waitForFinished();

}

void Caster::start()
{
    QString  cmd = QString("/usr/bin/ffmpeg -f alsa -ac 2 -i pulse -f x11grab -r 10 -s 1280x800 -i :0.0 -acodec aac -vcodec libx264 -preset ultrafast -crf 0 -threads 0 -f flv %1").
                    arg("rtmp://192.168.80.138/test/123");
    ffmpeg.start(cmd);
}

Caster::~Caster()
{
    delete ui;
}
