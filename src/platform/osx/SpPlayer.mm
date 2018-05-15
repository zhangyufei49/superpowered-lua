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
    if (_spPlayer->samplerate != samplerate) {
        LOGD("Samplerate from %u change to %u", _spPlayer->samplerate, samplerate);
        _spPlayer->samplerate = samplerate;
        _spPlayer->audioPlayer->setSamplerate(samplerate);
    };

    bool hasAudio = _spPlayer->audioPlayer->process(_spPlayer->stereoBuffer, false, numberOfSamples, _spPlayer->volume);

    if (hasAudio) {
        SuperpoweredDeInterleave(_spPlayer->stereoBuffer, outputBuffers[0], outputBuffers[1], numberOfSamples);
    }
    return hasAudio;
}

@end


static const unsigned int SAMPLERATE = 44100;

SpPlayer::SpPlayer(void* udata, superpower_player_event_callback callback)
        :SpPlayerBase(udata, callback, SAMPLERATE) {
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
