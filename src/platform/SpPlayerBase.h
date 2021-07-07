//
// Created by liushuai on 2018/5/8.
//

#ifndef SPPLAYERBASE_H
#define SPPLAYERBASE_H


#include "lua_superpowered_player.h"
#include "SuperpoweredAdvancedAudioPlayer.h"

/*
 * Superpowered播放器实现的基类
 * 需要不同的平台再去实现各自的音频IO并通过audioPlayer实现播放
 */
class SpPlayerBase {
public:
    SpPlayerBase(void* udata, superpower_player_event_callback callback, unsigned int samplerate);
    virtual ~SpPlayerBase() = 0;

    void playerEventCallback(SuperpoweredAdvancedAudioPlayerEvent event, void* value);

public:
    SuperpoweredAdvancedAudioPlayer* audioPlayer   = nullptr;
    superpower_player_event_callback eventCallback = nullptr;
    void* udata             = nullptr;
    unsigned int samplerate = 0;

    float volume        = 1.0f;
    bool  exitLoop      = false;
    bool  pauseWhenEOF  = true;
};

#endif //SPPLAYERBASE_H
