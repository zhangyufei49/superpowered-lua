//
// Created by liushuai on 18-4-24.
//

#ifndef ANDROID_MYLOG_H
#define ANDROID_MYLOG_H


#ifndef NDEBUG
    #if __ANDROID__
        #include <android/log.h>
        #define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, "Superpowered", __VA_ARGS__)
    #else
        #include <cstdio>
        #define LOGD(...)   do{printf("Superpowered: "); printf(__VA_ARGS__); printf("\n");} while(0)
    #endif
#else
    #define LOGD(...)       ((void)0)
#endif


#endif //ANDROID_MYLOG_H
