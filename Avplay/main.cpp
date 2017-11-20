#include"surface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //ffmpeg init
    av_register_all();
        avcodec_register_all();
        avformat_network_init();
    Surface w;
    w.show();

    return a.exec();
}
