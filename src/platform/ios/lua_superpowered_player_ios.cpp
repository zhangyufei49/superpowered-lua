//
// Created by liushuai on 18-5-2.
//

#include <string>
#include <cassert>

#include "lua_superpowered_player.h"
#include "SpPlayer.h"
#include "mylog.h"

// cocos2d
#include "platform/CCFileUtils.h"
#include "CCDirector.h"


void* superpowered_player_create(superpower_player_event_callback callback, void* udata) {
    auto player = new SpPlayer(udata, callback);
    LOGD("superpowered_player_create:player:%p, udata:%p", player, udata);
    return player;
}

void superpowered_player_open(void* player, const char* filePath) {
    LOGD("superpowered_player_open:player=%p, path=%s", player, filePath);

    std::string path = filePath;

    if (path[0] != '/') {
        // 非绝对路径时 使用cocos的fullPathForFilename进一步确定路径
        path = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
    }

    if (path[0] != '/') {
        // iOS里都是绝对路径
        LOGD("can not find file:%s", path.c_str());
        assert(false);
    }
    else {
        // 打开绝对路径
        ((SpPlayer*)player)->audioPlayer->open(path.c_str());
    }
}
