//
// Created by liushuai on 18-5-8.
//

#include "SpPlayer.h"
#include "mylog.h"
#include "SuperpoweredSimple.h"
#include "SuperpoweredIOSAudioIO.h"


@interface AudioIODelegate : NSObject <SuperpoweredIOSAudioIODelegate> {

}

@end

@implementation AudioIODelegate

- (void)interruptionStarted {

}

- (void)interruptionEnded {

}

- (void)recordPermissionRefused {

}

- (void)mapChannels:(multiOutputChannelMap *)outputMap inputMap:(multiInputChannelMap *)inputMap externalAudioDeviceName:(NSString *)externalAudioDeviceName outputsAndInputs:(NSString *)outputsAndInputs {

}

@end


static AudioIODelegate* delegate = [AudioIODelegate new];

static const unsigned int SAMPLERATE = 44100;      // 在iOS平台 使用其他采样率会有问题

SpPlayer::SpPlayer(void* udata, superpower_player_event_callback callback)
        :SpPlayerBase(udata, callback, SAMPLERATE) {
    LOGD("SpPlayer::SpPlayer udata=%p", udata);

    if (posix_memalign((void **)&stereoBuffer, 16, 4096 + 128) != 0) abort();   // Allocating memory, aligned to 16.

    // NOTE(liushuai): 要保证在AudioIO调stop后audioProcessing不会再被回调 当前未添加保护的逻辑 暂时认为库本身做到了这一点
    auto audioProcessing = [](void *self,  float **buffers, unsigned int inputChannels, unsigned int outputChannels, unsigned int numberOfSamples, unsigned int samplerate, uint64_t hostTime) {
        auto spPlayer = (SpPlayer*)self;

        if (spPlayer->samplerate != samplerate) {
            LOGD("Samplerate from %u change to %u", spPlayer->samplerate, samplerate);
            spPlayer->samplerate = samplerate;
            spPlayer->audioPlayer->setSamplerate(samplerate);
        };

        bool hasAudio = spPlayer->audioPlayer->process(spPlayer->stereoBuffer, false, numberOfSamples, spPlayer->volume);
        if (hasAudio) SuperpoweredDeInterleave(spPlayer->stereoBuffer, buffers[0], buffers[1], numberOfSamples);
        return hasAudio;
    };

    SuperpoweredIOSAudioIO* iosAudioIO = [[SuperpoweredIOSAudioIO alloc] initWithDelegate:delegate preferredBufferSize:12
                                                    preferredMinimumSamplerate:SAMPLERATE
                                                          audioSessionCategory:AVAudioSessionCategoryPlayback
                                                                      channels:2
                                                       audioProcessingCallback:audioProcessing
                                                                    clientdata:this];
    [iosAudioIO start];
    audioIO = (__bridge_retained void*)iosAudioIO;
}

SpPlayer::~SpPlayer() {
    LOGD("SpPlayer::~SpPlayer this=%p", this);

    SuperpoweredIOSAudioIO* iosAudioIO = (__bridge_transfer SuperpoweredIOSAudioIO*)audioIO;
    [iosAudioIO stop];
    iosAudioIO = nil;
    free(stereoBuffer);
}
