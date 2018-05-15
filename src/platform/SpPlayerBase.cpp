//
// Created by liushuai on 2018/5/8.
//

#include "SpPlayerBase.h"
#include "mylog.h"

#ifdef USE_COCOS2D
// cocos2d
#include "CCDirector.h"
#include "CCScheduler.h"
#endif

SpPlayerBase::SpPlayerBase(void* udata, superpower_player_event_callback callback, unsigned int samplerate)
        :udata(udata), eventCallback(callback), samplerate(samplerate) {
    LOGD("SpPlayerBase::SpPlayerBase this=%p", this);

    audioPlayer = new SuperpoweredAdvancedAudioPlayer(udata, [](void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
        auto spPlayer = (SpPlayerBase*)((LuaPlayer*)clientData)->player;
        spPlayer->playerEventCallback(event, value);
    }, samplerate, 4);

    audioPlayer->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
}

SpPlayerBase::~SpPlayerBase() {
    LOGD("SpPlayerBase::~SpPlayerBase this=%p", this);
    delete audioPlayer;
}

void SpPlayerBase::playerEventCallback(SuperpoweredAdvancedAudioPlayerEvent event, void* value) {
    LOGD("SpPlayerBase:%p, event:%d", this, event);

    switch (event) {
        case SuperpoweredAdvancedAudioPlayerEvent_EOF:
            *((bool*)value) = this->pauseWhenEOF;
            break;
        case SuperpoweredAdvancedAudioPlayerEvent_LoopEnd:
            *((bool*)value) = this->exitLoop;
            break;
        default:
            break;
    }

    if (this->eventCallback) {
#if USE_COCOS2D
        // NOTE(liushuai): 要保证这个回调在lua线程 eventCallback是lua传过来的
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
#endif
            this->eventCallback(this->udata, event, value);
#if USE_COCOS2D
        });
#endif
    }
}
