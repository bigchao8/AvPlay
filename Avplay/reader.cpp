#include "reader.h"

Reader::Reader(QString filename, QObject *parent) :
    QObject(parent)
{
 this->filename=filename;
    flag=true;
    pthread_create(&tid,NULL,thread_func,this);
}
void* Reader::thread_func(void *ptr)
{
   Reader* This = (Reader*)ptr;
    if(This->init())
        This->run();
    This->fini();
    return NULL;
}
bool Reader::init()
{
    //open file
    bool ret=open();
    if(!ret) return ret;

    //get stream info
    ret=getStreamInfo();
    if(!ret) return ret;

    //open Decoder
    ret=getDecoder();
    if(!ret) return ret;

    //init audioOutput
    ret=initAudioOutput();
    if(!ret) return ret;

    //init Translate
    ret=initTranslate();
    if(!ret) return ret;

    return true;


}
bool Reader::open()
{

    int ret=avformat_open_input(&this->inputFormatCtx,
                                this->filename.toLocal8Bit().data(),
                                NULL,NULL);
    if(ret != 0)
       {
           qDebug() << "open file error";
           return false;
       }

    return true;
}

bool Reader::getStreamInfo()
{
    //find stream info
  int ret=avformat_find_stream_info(this->inputFormatCtx,NULL);
  if(ret != 0)
      {
          qDebug() << "error find stream info";
          return false;
      }

  //get stream type
  for(unsigned  int i=0;i<inputFormatCtx->nb_streams;++i)
  {
      AVStream* stream=inputFormatCtx->streams[i];
     if(stream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
     {
         this->audioIndex=i;
         audioStream=stream;
     }else if(stream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
     {
         this->videoIndex=i;
         videoStream=stream;
     }
  }

  if(audioIndex == -1 && videoIndex ==-1)
      {
          qDebug() << "can not find media index";
          return false;
      }
      return true;


}

bool Reader::getDecoder()
{
    //Find a registered decoder with a matching codec ID.
  audioCodec=avcodec_find_decoder(audioStream->codecpar->codec_id);

  //allocate context
  audioCodecCtx=avcodec_alloc_context3(audioCodec);

  //Fill context form codec values
  avcodec_parameters_to_context(audioCodecCtx,audioStream->codecpar);

  //Initialize the AVCodecContext to use the given AVCodec.Prior to using this
  int ret = avcodec_open2(audioCodecCtx, audioCodec, NULL);
      if(ret != 0)
      {
          qDebug() << "open audio codec error";
          return false;
      }

      videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
         videoCodecCtx = avcodec_alloc_context3(videoCodec);
         avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar);
         ret = avcodec_open2(videoCodecCtx, videoCodec, NULL);
         if(ret != 0)
         {
             qDebug() << "open video codec error";
             return false;
         }

         return true;

}

bool Reader::initAudioOutput()
{
    //Byte Order
  format.setByteOrder(QAudioFormat::LittleEndian);
  //Channel count audio
  format.setChannelCount(audioCodecCtx->channels);
  //set codec
  format.setCodec("audio/pcm");
  //set sample rate
  format.setSampleRate(audioCodecCtx->sample_rate);
  //set sample size
  format.setSampleSize(32);
  //set sample type
  format.setSampleType(QAudioFormat::Float);


  //get outputdevice info
  audioInfo=QAudioDeviceInfo::defaultOutputDevice();

  //should supported format form audio format
  if(!audioInfo.isFormatSupported(format))
     {
         format = audioInfo.nearestFormat(format);
     }
  this->audioOutput=new QAudioOutput(format);
  this->audioIO=this->audioOutput->start();
  return true;
}

bool Reader::initTranslate()
{
    //get cached context
 sws=sws_getCachedContext(NULL,
                          videoCodecCtx->width,
                          videoCodecCtx->height,
                          videoCodecCtx->pix_fmt,
                          videoCodecCtx->width,
                          videoCodecCtx->height,
                          AV_PIX_FMT_RGBA,
                          SWS_BICUBIC,
                          NULL,NULL,NULL);

 videoBuff=QImage(videoCodecCtx->width,
                  videoCodecCtx->height,
                  QImage::Format_RGBA8888);
 //
 AVSampleFormat out_sample_fmt=getSampleFormat();

 if(out_sample_fmt == AV_SAMPLE_FMT_NONE)
   {
       qDebug() << "sample_fmt trans error";
       return false;
   }

 swr=swr_alloc_set_opts(NULL,
                        av_get_default_channel_layout(
                            format.channelCount()),
                        out_sample_fmt,
                        format.sampleRate()/speed,
                        audioCodecCtx->channel_layout,
                        audioCodecCtx->sample_fmt,
                        audioCodecCtx->sample_rate,0,NULL);

 swr_init(swr);
 audioBuff=new char[256*1024];
 return true;

}

AVSampleFormat Reader::getSampleFormat()
{
    if(format.sampleSize() == 8
             && format.sampleType() == QAudioFormat::UnSignedInt)
     {
         return AV_SAMPLE_FMT_U8;
     }
     else if(format.sampleSize() == 16
             && format.sampleType() == QAudioFormat::SignedInt)
     {
         return AV_SAMPLE_FMT_S16;
     }
     else if(format.sampleSize() == 32
             && format.sampleType() == QAudioFormat::SignedInt)
     {
         return AV_SAMPLE_FMT_S32;
     }
     else if(format.sampleSize() == 32
             && format.sampleType() == QAudioFormat::Float)
     {
         return AV_SAMPLE_FMT_FLT;
     }
     return AV_SAMPLE_FMT_NONE;
 return AV_SAMPLE_FMT_NONE;
}

void Reader::run()
{
 pkt=av_packet_alloc();
 frm=av_frame_alloc();
 while(flag)
 {
     int ret=av_read_frame(inputFormatCtx,pkt);
     if(ret != 0)
             {
                 qDebug() << "error or EOF";
                 break;
             }

     if(pkt->stream_index == videoIndex)
            {
                handleVideoPacket();
            }

     else if(pkt->stream_index == audioIndex)
            {
         handleAudioPacket();
            }
     av_packet_unref(pkt);
 }
}

void Reader::handleAudioPacket()
{
    //
 avcodec_send_packet(audioCodecCtx,pkt);
 avcodec_receive_frame(audioCodecCtx,frm);

 int sample_count=swr_convert(swr, (uint8_t **)&audioBuff, frm->nb_samples*2,
                                        (const uint8_t **)frm->data, frm->nb_samples);

 int bytes=sample_count*format.channelCount()*format.sampleSize()/8;

 doWrite(bytes);


}

void Reader::doWrite(int bytes)
{
  int alreadyWrite=0;
  do
  {
      int ret=this->audioIO->write(this->audioBuff+alreadyWrite,
                                   bytes-alreadyWrite);
       if(ret > 0) alreadyWrite += ret;
        else if(ret == 0) QThread::msleep(frm->nb_samples*1000.0/format.sampleRate()-5);
       else break;
          }while(alreadyWrite < bytes);
}
void Reader::handleVideoPacket()
{
    avcodec_send_packet(videoCodecCtx, pkt);
        avcodec_receive_frame(videoCodecCtx, frm);

        uint8_t* data[1] = {videoBuff.bits()};
        int linesize[1] = {videoBuff.width()*4};
        sws_scale(sws, frm->data, frm->linesize, 0, frm->height,
                 data, linesize);

        emit sigNewImage(videoBuff);
}

void Reader::fini()
{
    if(inputFormatCtx ) avformat_close_input(&inputFormatCtx);
       if(audioCodecCtx) avcodec_free_context(&audioCodecCtx);
       if(videoCodecCtx) avcodec_free_context(&videoCodecCtx);
       if(swr) swr_free(&swr);
       if(sws) sws_freeContext(sws);
       if(audioBuff) delete []audioBuff;
       if(pkt) av_packet_free(&pkt);
       if(frm)av_frame_free(&frm);
       if(audioIO) audioIO->close();
       if(audioOutput) delete audioOutput;
}

void Reader::quit()
{
    flag = false;
       pthread_join(tid, NULL);
}
