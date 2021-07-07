//
// Created by liushuai on 2018/5/8.
//

#include "lua_superpowered_player.h"
#include "SpPlayerBase.h"


/******************************* 平台相关实现 ********************************/
// 不同平台有不同的open资源和创建player的方式 这部分在平台相关的文件实现
extern void* superpowered_player_create(superpower_player_event_callback callback, void* udata);
extern void superpowered_player_open(void* player, const char* filePath);


/******************************* 平台通用实现 ********************************/
void superpowered_player_release(void* player) {
    delete (SpPlayerBase*)player;
}

void* superpowered_player_get(void* player) {
    return ((SpPlayerBase*)player)->audioPlayer;
}

float superpowered_player_volume(void* player) {
    return ((SpPlayerBase*)player)->volume;
}

void superpowered_player_set_volume(void* player, float volume) {
    ((SpPlayerBase*)player)->volume = volume;
}

void superpowered_player_set_exit_loop(void* player, int exit) {
    ((SpPlayerBase*)player)->exitLoop = (bool) exit;
}

void superpowered_player_set_pause_when_eof(void* player, int pause) {
    ((SpPlayerBase*)player)->pauseWhenEOF = (bool) pause;
}
