//
// Created by liushuai on 18-5-8.
//

#ifndef SPPLAYER_H
#define SPPLAYER_H


#include "SpPlayerBase.h"

class SpPlayer : public SpPlayerBase {
public:
    explicit SpPlayer(void* udata, superpower_player_event_callback callback);
    ~SpPlayer();

private:
    void*  audioIO      = nullptr;
    float* stereoBuffer = nullptr;
};


#endif //SPPLAYER_H
