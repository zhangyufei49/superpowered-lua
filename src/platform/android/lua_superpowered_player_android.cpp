//
// Created by liushuai on 18-5-2.
//

#include <string>
#include <cassert>
#include <android/asset_manager.h>

#include "lua_superpowered_player.h"
#include "SpPlayer.h"
#include "mylog.h"

// cocos2d
#include "platform/android/CCFileUtils-android.h"
#include "platform/CCFileUtils.h"
#include "CCDirector.h"


void* superpowered_player_create(superpower_player_event_callback callback, void* udata) {
    auto player = new SpPlayer(udata, callback);
    LOGD("superpowered_player_create:player:%p, udata:%p", player, udata);
    return player;
}

// cocos里的一个全局变量(Android apk路径)
extern std::string g_apkPath;

// 平台相关的打开函数 对于Android平台要处理Assert里的资源路径
void superpowered_player_open(void* player, const char* filePath) {
    LOGD("superpowered_player_open:player=%p, path=%s", player, filePath);

    std::string path = filePath;

    if (path[0] != '/') {
        // 非绝对路径时 使用cocos的fullPathForFilename进一步确定路径
        path = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
    }

    if (path[0] != '/') {
        // 不是绝对路径时 就只可能是assert中的资源了
        std::string relativePath;
        // 删除"assets/"前缀
        size_t position = path.find("assets/");
        if (0 == position) {
            relativePath += path.substr(strlen("assets/"));
        } else {
            relativePath += path;
        }

        AAsset* asset = AAssetManager_open(cocos2d::FileUtilsAndroid::getAssetManager(), relativePath.c_str(), AASSET_MODE_UNKNOWN);
        off_t start, length;
        const int _assetFd = AAsset_openFileDescriptor(asset, &start, &length);
        AAsset_close(asset);
        LOGD("_assetFd:%d, start:%ld, length:%ld", _assetFd, start, length);
        if (_assetFd < 0) {
            LOGD("_assetFd<0!!!");
            assert(false);
            return;
        }

        ((SpPlayer*)player)->audioPlayer->open(g_apkPath.c_str(), start, length);
    }
    else {
        // 打开绝对路径
        ((SpPlayer*)player)->audioPlayer->open(path.c_str());
    }
}
