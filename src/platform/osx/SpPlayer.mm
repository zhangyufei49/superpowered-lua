//
// Created by liushuai on 18-5-11.
//

#include "SpPlayer.h"
#include "mylog.h"
#include "SuperpoweredSimple.h"
#include "SuperpoweredOSXAudioIO.h"


@interface AudioIODelegate : NSObject <SuperpoweredOSXAudioIODelegate>

@property(nonatomic, assign) SpPlayer* spPlayer;

@end

@implementation AudioIODelegate

- (bool)audioProcessingCallback:(float**)inputBuffers
                  inputChannels:(unsigned int)inputChannels
                  outputBuffers:(float**)outputBuffers
                 outputChannels:(unsigned int)outputChannels
                numberOfSamples:(unsigned int)numberOfSamples
                     samplerate:(unsigned int)samplerate
                       hostTime:(UInt64)hostTime {
    bool silence = !_spPlayer->audioPlayer->process(_spPlayer->stereoBuffer, false, numberOfSamples, _spPlayer->volume);

    if (!silence) {
        SuperpoweredDeInterleave(_spPlayer->stereoBuffer, outputBuffers[0], outputBuffers[1], numberOfSamples);
    }
    return !silence;
}

@end


static const unsigned int samplerate = 44100;      // 在iOS平台 使用其他采样率会有问题

SpPlayer::SpPlayer(void* udata, superpower_player_event_callback callback)
        :SpPlayerBase(udata, callback, samplerate) {
    LOGD("SpPlayer::SpPlayer udata=%p", udata);

    if (posix_memalign((void **)&stereoBuffer, 16, 4096 + 128) != 0) abort();   // Allocating memory, aligned to 16.

    AudioIODelegate* delegate = [AudioIODelegate new];
    delegate.spPlayer = this;
    SuperpoweredOSXAudioIO* osxAudioIO = [[SuperpoweredOSXAudioIO alloc] initWithDelegate:delegate
                                                                    preferredBufferSizeMs:12
                                                                         numberOfChannels:2
                                                                              enableInput:false
                                                                             enableOutput:true];
    [osxAudioIO start];
    this->audioIO  = (__bridge_retained void*)osxAudioIO;
    this->delegate = (__bridge_retained void*)delegate;
}

SpPlayer::~SpPlayer() {
    LOGD("SpPlayer::~SpPlayer this=%p", this);

    SuperpoweredOSXAudioIO* osxAudioIO = (__bridge_transfer SuperpoweredOSXAudioIO*)audioIO;
    [osxAudioIO stop];
    osxAudioIO = nil;

    AudioIODelegate* delegate = (__bridge_transfer AudioIODelegate*)this->delegate;
    delegate = nil;
    free(stereoBuffer);
}
