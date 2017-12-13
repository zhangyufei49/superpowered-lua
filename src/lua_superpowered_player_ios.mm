#include <stdlib.h>
#include "lua_superpowered_player.h"
#include "Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "Superpowered/SuperpoweredSimple.h"
#import "Superpowered/SuperpoweredIOSAudioIO.h"

#pragma mark - AudioPlayer

#define SAMPLERATE 44100

@interface AudioPlayer : NSObject<SuperpoweredIOSAudioIODelegate>

@property (nonatomic, assign) SuperpoweredAdvancedAudioPlayer* player;
@property (nonatomic, assign) float volume;
@property (nonatomic, assign) BOOL exitLoop;
@property (nonatomic, assign) BOOL pauseWhenEOF;

@end

@implementation AudioPlayer {
    void* udata;
    superpower_player_event_callback callback;
    unsigned int lastSamplerate;
    SuperpoweredIOSAudioIO* output;
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

static bool audioPlayerProcessing(void* clientdata, float** buffers, unsigned int inputChannels, unsigned int outputChannels,
                                  unsigned int numberOfSamples, unsigned int samplerate, uint64_t hostTime) {
    __unsafe_unretained AudioPlayer* ap = (__bridge AudioPlayer*)clientdata;
    if (samplerate != ap->lastSamplerate) {
        ap->lastSamplerate = samplerate;
        ap.player->setSamplerate(samplerate);
    }

    bool silence = !ap.player->process(ap->stereoBuffer, false, numberOfSamples, ap.volume);

    if (!silence) SuperpoweredDeInterleave(ap->stereoBuffer, buffers[0], buffers[1], numberOfSamples);
    return !silence;
}

- (id)initWithCallback:(void (*)(void* udata, int event, void* value))callback udata:(void*)udata {
    if (self = [super init]) {
        _pauseWhenEOF = YES;
        _exitLoop     = NO;
        _volume       = 1.0f;

        self->udata        = udata;
        self->callback     = callback;

        if (posix_memalign((void**)&(self->stereoBuffer), 16, 4096 + 128) != 0) {
            abort();
        }

        _player           = new SuperpoweredAdvancedAudioPlayer((__bridge void*)self, playerEventCallback, SAMPLERATE, 0);
        _player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

        output = [[SuperpoweredIOSAudioIO alloc] initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self
                                              preferredBufferSize:12
                                       preferredMinimumSamplerate:SAMPLERATE
                                             audioSessionCategory:AVAudioSessionCategoryPlayback
                                                         channels:2
                                          audioProcessingCallback:audioPlayerProcessing
                                                       clientdata:(__bridge void*)self];
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

- (void)interruptionEnded {
    _player->onMediaserverInterrupt();
    _player->onMediaserverInterrupt();
}

- (void)interruptionStarted {
}

- (void)mapChannels:(multiOutputChannelMap*)outputMap
                   inputMap:(multiInputChannelMap*)inputMap
    externalAudioDeviceName:(NSString*)externalAudioDeviceName
           outputsAndInputs:(NSString*)outputsAndInputs {
}

- (void)recordPermissionRefused {
}

@end

#pragma mark - public interfaces

void* superpowered_player_create(superpower_player_event_callback callback, void* udata) {

}

void superpowered_player_release(void* player) {
}

void* superpowered_player_get(void* player) {
    return NULL;
}

float superpowered_player_volume(void* player) {
    return 0;
}

void superpowered_player_set_volume(void* player, float volume) {
}
