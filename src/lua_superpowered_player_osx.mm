#include <stdlib.h>
#include "lua_superpowered_player.h"
#include "Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "Superpowered/SuperpoweredSimple.h"
#import "Superpowered/SuperpoweredOSXAudioIO.h"

#pragma mark - AudioPlayer

#define SAMPLERATE 44100

@interface AudioPlayer : NSObject<SuperpoweredOSXAudioIODelegate>

@property(nonatomic, assign) SuperpoweredAdvancedAudioPlayer* player;
@property(nonatomic, assign) float volume;
@property(nonatomic, assign) BOOL exitLoop;
@property(nonatomic, assign) BOOL pauseWhenEOF;

@end

@implementation AudioPlayer {
    void* udata;
    superpower_player_event_callback callback;
    unsigned int lastSamplerate;
    SuperpoweredOSXAudioIO* output;
    float* stereoBuffer;
}

static void playerEventCallback(void* clientData, SuperpoweredAdvancedAudioPlayerEvent event, void* value) {
    AudioPlayer* ap = (__bridge AudioPlayer*)clientData;
    switch (event) {
    case SuperpoweredAdvancedAudioPlayerEvent_EOF:
        *((bool*)value) = ap.pauseWhenEOF ? true : false;
        break;
    case SuperpoweredAdvancedAudioPlayerEvent_LoopEnd:
        *((bool*)value) = ap.exitLoop ? false : true;
        break;
    default:
        break;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        if (ap->callback) {
            ap->callback(ap->udata, event, value);
        }
    });
}

- (bool)audioProcessingCallback:(float**)inputBuffers
                  inputChannels:(unsigned int)inputChannels
                  outputBuffers:(float**)outputBuffers
                 outputChannels:(unsigned int)outputChannels
                numberOfSamples:(unsigned int)numberOfSamples
                     samplerate:(unsigned int)samplerate
                       hostTime:(UInt64)hostTime {
    if (samplerate != self->lastSamplerate) {
        self->lastSamplerate = samplerate;
        _player->setSamplerate(samplerate);
    }

    bool silence = !_player->process(self->stereoBuffer, false, numberOfSamples, _volume);

    if (!silence) {
        SuperpoweredDeInterleave(self->stereoBuffer, outputBuffers[0], outputBuffers[1], numberOfSamples);
    }
    return !silence;
}

- (id)initWithCallback:(void (*)(void* udata, int event, void* value))callback udata:(void*)udata {
    if (self = [super init]) {
        _pauseWhenEOF = YES;
        _exitLoop     = NO;
        _volume       = 1.0f;

        self->udata    = udata;
        self->callback = callback;

        if (posix_memalign((void**)&(self->stereoBuffer), 16, 4096 + 128) != 0) {
            abort();
        }

        _player           = new SuperpoweredAdvancedAudioPlayer((__bridge void*)self, playerEventCallback, SAMPLERATE, 0);
        _player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

        output = [[SuperpoweredOSXAudioIO alloc] initWithDelegate:(id<SuperpoweredOSXAudioIODelegate>)self
                                            preferredBufferSizeMs:12
                                                 numberOfChannels:2
                                                      enableInput:false
                                                     enableOutput:true];
        [output start];
    }
    return self;
}

- (void)dealloc {
    [output stop];
    delete _player;
    free(stereoBuffer);
#if !__has_feature(objc_arc)
    [output release];
    [super dealloc];
#endif
}

@end

void* superpowered_player_create(superpower_player_event_callback callback, void* udata) {
#if !__has_feature(objc_arc)
    AudioPlayer* p = [[AudioPlayer alloc] initWithCallback:callback udata:udata];
    return (__bridge void*)p;
#else
    AudioPlayer* p = [[AudioPlayer alloc] initWithCallback:callback udata:udataa];
    return (__bridge_retained void*)p;
#endif
}

void superpowered_player_release(void* player) {
#if !__has_feature(objc_arc)
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    [p dealloc];
#else
    AudioPlayer* p = (__bridge_transfer AudioPlayer*)player;
    p              = nil;
#endif
}

void* superpowered_player_get(void* player) {
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    return (void*)p.player;
}

float superpowered_player_volume(void* player) {
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    return p.volume;
}

void superpowered_player_set_volume(void* player, float volume) {
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    p.volume       = volume;
}

void superpowered_player_set_exit_loop(void* player, int exit) {
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    p.exitLoop     = exit ? YES : NO;
}

void superpowered_player_set_pause_when_eof(void* player, int pause) {
    AudioPlayer* p = (__bridge AudioPlayer*)player;
    p.pauseWhenEOF = pause ? YES : NO;
}
