#ifndef READER_H
#define READER_H

#include <QObject>
extern "C"
{
// 格式相关的头文件
#include <libavformat/avformat.h>
// 编解码相关的头文件
#include <libavcodec/avcodec.h>
// 图片格式转换的头文件
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}
#include <QImage>
#include <pthread.h>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QDebug>
#include <QThread>
class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(QString filename, QObject *parent = 0);
    void quit();

private:
    QString filename;
    bool flag;
    pthread_t tid;

    //play audio p

    AVFormatContext * inputFormatCtx=NULL;
    /*
    流结构。
    新字段可以添加到末尾，带有小的版本颠簸。
    删除、重新排序和更改现有字段需要一个专业
    版本肿块。
    sizeof(AVStream)不得在libav *外使用。
    */
    AVStream* audioStream=NULL;
    AVStream* videoStream=NULL;
    //av codec
    AVCodec* audioCodec=NULL;
    AVCodec* videoCodec=NULL;
    //av codec context=
    AVCodecContext* audioCodecCtx=NULL;
    AVCodecContext* videoCodecCtx=NULL;
    //Swr sws
    SwrContext* swr=NULL;
    SwsContext* sws=NULL;
    //image pinter
    QImage videoBuff;
    char * audioBuff;

    //packet frame
    AVPacket* pkt=NULL;
    AVFrame* frm=NULL;

    //av index
    int videoIndex=-1;
    int audioIndex=-1;
    //av player speed
    float speed=1.0;
    //av output device
    QAudioOutput* audioOutput=NULL;
    //io
    QIODevice* audioIO=NULL;
    //audio format
    QAudioFormat format;
    //audio Device info
    QAudioDeviceInfo audioInfo;

    //sample format
    AVSampleFormat getSampleFormat();
    static void* thread_func(void* ptr);

    bool init();
    void fini();
    void run();

    bool open();
    bool getStreamInfo();
    bool getDecoder();
    bool initAudioOutput();
    bool initTranslate();

    void handleVideoPacket();
    void handleAudioPacket();

    void doWrite(int bytes);
signals:
void sigNewImage(QImage image);
public slots:
};

#endif // READER_H
