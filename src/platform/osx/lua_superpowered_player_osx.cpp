//
// Created by liushuai on 18-5-11.
//

#include "lua_superpowered_player.h"
#include "SpPlayer.h"
#include "mylog.h"

void* superpowered_player_create(superpower_player_event_callback callback, void* udata) {
    auto player = new SpPlayer(udata, callback);
    LOGD("superpowered_player_create:player:%p, udata:%p", player, udata);
    return player;
}

void superpowered_player_open(void* player, const char* filePath) {
    LOGD("superpowered_player_open:player=%p, path=%s", player, filePath);

    // 打开绝对路径
    ((SpPlayer*)player)->audioPlayer->open(filePath);
}
