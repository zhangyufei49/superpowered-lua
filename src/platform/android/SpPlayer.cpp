//
// Created by liushuai on 18-5-2.
//

#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES.h>
#include <malloc.h>

#include "SpPlayer.h"
#include "SuperpoweredSimple.h"
#include "mylog.h"


SpPlayer::SpPlayer(void* udata, superpower_player_event_callback callback, unsigned int samplerate, unsigned int buffersize)
        :SpPlayerBase(udata, callback, samplerate) {
    LOGD("SpPlayer::SpPlayer this=%p, udata=%p", this, udata);

    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, [](void* self, short int* audioIO, int numberOfSamples, int samplerate) {
        auto spPlayer = (SpPlayer*)self;

        bool hasAudio = spPlayer->audioPlayer->process(spPlayer->stereoBuffer, false, (unsigned int)numberOfSamples, spPlayer->volume);
        if (hasAudio) SuperpoweredFloatToShortInt(spPlayer->stereoBuffer, audioIO, (unsigned int)numberOfSamples);
        return hasAudio;
    }, this, -1, SL_ANDROID_STREAM_MEDIA);
}

SpPlayer::~SpPlayer() {
    LOGD("SpPlayer::~SpPlayer this=%p", this);

    delete audioIO;
    free(stereoBuffer);
}
