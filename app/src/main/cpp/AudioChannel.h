//
// Created by wanglei55 on 2019/4/30.
//


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext);
    ~AudioChannel();

private:

};

