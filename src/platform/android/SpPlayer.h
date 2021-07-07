//
// Created by liushuai on 18-4-24.
//

#ifndef SPPLAYER_H
#define SPPLAYER_H


#include "SpPlayerBase.h"
#include "AndroidIO/SuperpoweredAndroidAudioIO.h"

class SpPlayer : public SpPlayerBase {
public:
    SpPlayer(void* udata, superpower_player_event_callback callback, unsigned int samplerate = 48000, unsigned int buffersize = 512);
    ~SpPlayer();

private:
    SuperpoweredAndroidAudioIO* audioIO = nullptr;
    float* stereoBuffer                 = nullptr;
};


#endif //SPPLAYER_H
