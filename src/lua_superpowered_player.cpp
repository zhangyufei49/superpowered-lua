#include "lua_superpowered_player.h"
#include "Superpowered/SuperpoweredAdvancedAudioPlayer.h"

#define SUPERPOWERED_PLAYER_CALLBACKS "superpowered_player_callbacks"
#define LOG_ID "superpowered_player"

#define GET_SPPLAYER_FROM_LUA_PLAYER(_spplayer_name_, _lua_player_) \
    SuperpoweredAdvancedAudioPlayer* _spplayer_name_ = (SuperpoweredAdvancedAudioPlayer*)(superpowered_player_get(_lua_player_->player))

#define CHECK_ARG_IS_STH(_lua_state_, _n_, _type_)                                                                   \
    if (!lua_is##_type_(l, _n_)) {                                                                                   \
        fprintf(stderr, "[%s] invalid argument when call %s, argv %d need a %s.\n", LOG_ID, __func__, _n_, #_type_); \
        return 0;                                                                                                    \
    }

typedef struct {
    void* player;
    lua_State* l;
} LuaPlayer;

static LuaPlayer* create_lua_player(lua_State* l) {
    LuaPlayer* p = new LuaPlayer;
    p->l         = l;

    // reg call back
    lua_pushstring(l, SUPERPOWERED_PLAYER_CALLBACKS);
    lua_rawget(l, LUA_REGISTRYINDEX);
    lua_pushinteger(l, (lua_Integer)p);
    lua_pushvalue(l, 1);
    lua_rawset(l, -3);
    lua_pop(l, 1);

    return p;
}

static void release_lua_player(LuaPlayer* p) {
    lua_State* l = p->l;
    lua_pushstring(l, SUPERPOWERED_PLAYER_CALLBACKS);
    lua_rawget(l, LUA_REGISTRYINDEX);
    lua_pushinteger(l, (lua_Integer)p);
    lua_pushnil(l);
    lua_rawset(l, -3);
    lua_pop(l, 1);

    delete p;
}

static void player_event_callback(void* udata, SuperpoweredAdvancedAudioPlayerEvent event, void* value) {
    LuaPlayer* p = static_cast<LuaPlayer*>(udata);
    lua_State* l = p->l;

    lua_pushstring(l, SUPERPOWERED_PLAYER_CALLBACKS);
    lua_rawget(l, LUA_REGISTRYINDEX);
    lua_pushinteger(l, (lua_Integer)p);
    lua_rawget(l, -2);

    lua_pushlightuserdata(l, p);
    lua_pushinteger(l, (lua_Integer)event);
    switch (event) {
    case SuperpoweredAdvancedAudioPlayerEvent_JogParameter:
        lua_pushnumber(l, (lua_Number)(*((double*)value)));
        break;
    case SuperpoweredAdvancedAudioPlayerEvent_LoadError:
        lua_pushstring(l, (const char*)value);
        break;
    // NOTE(zhangyufei):
    // DurrationChanged and LoadSuccess return nil
    // EOF and LoopEnd are handle by platform. Just push nil to lua here.
    // HLSNetworkError and ProgressiveDownloadError only occur when load source from network and I don't want to support it.
    default:
        lua_pushnil(l);
    }
    lua_call(l, 3, 0);
}

static LuaPlayer* check_arg1_is_lua_player(lua_State* l, const char* funcname, int needargc) {
    int argc = lua_gettop(l);
    if ((argc != needargc || !lua_islightuserdata(l, 1))) {
        fprintf(stderr, "[%s] invalid argument when call %s, and it needs %d arguments\n", LOG_ID, funcname, needargc);
        return NULL;
    }

    return static_cast<LuaPlayer*>(lua_touserdata(l, 1));
}

static int player_create(lua_State* l) {
    // check arg
    int argc = lua_gettop(l);
    if (argc != 1 || !lua_isfunction(l, 1)) {
        fprintf(stderr, "[%s] %s need a event callback argument\n", LOG_ID, __func__);
        return 0;
    }

    // create player
    LuaPlayer* p = create_lua_player(l);
    p->player = superpowered_player_create((superpower_player_event_callback)player_event_callback, p);
    lua_pushlightuserdata(l, p);
    return 1;
}

static int player_release(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        superpowered_player_release(p->player);
        release_lua_player(p);
    }

    return 0;
}

static int player_duration(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->durationMs);
        return 1;
    }
    return 0;
}

static int player_position(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->positionMs);
        return 1;
    }
    return 0;
}

static int player_tempo(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->tempo);
        lua_pushboolean(l, player->masterTempo ? 1 : 0);
        return 2;
    }
    return 0;
}

static int player_playing(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushboolean(l, player->playing ? 1 : 0);
        return 1;
    }
    return 0;
}

static int player_looping(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushboolean(l, player->looping ? 1 : 0);
        return 1;
    }
    return 0;
}

static int player_volume(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);
    if (p) {
        lua_pushnumber(l, (lua_Number)superpowered_player_volume(p->player));
        return 1;
    }
    return 0;
}

static int player_open(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 2);

    // check argv 2 is string
    CHECK_ARG_IS_STH(l, 2, string)

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        const char* path = lua_tostring(l, 2);
        player->open(path);
    }
    return 0;
}

static int player_play(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        player->play(false);
    }

    return 0;
}

static int player_pause(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 1);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        player->pause();
    }

    return 0;
}

static int player_seek(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 3);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number)
    // check argv 3 is boolean
    CHECK_ARG_IS_STH(l, 3, boolean)

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number pos = lua_tonumber(l, 2);
        bool stop      = lua_toboolean(l, 3) ? true : false;
        // FIXME(zhangyufei): 参数3 我没搞明白。这里暂时用 false
        player->setPosition(pos, stop, false);
    }

    return 0;
}

static int player_loop(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 4);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number)
    // check argv 3 is number
    CHECK_ARG_IS_STH(l, 3, number)
    // check argv 4 is boolean
    CHECK_ARG_IS_STH(l, 4, boolean)

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number start = lua_tonumber(l, 2);
        lua_Number len   = lua_tonumber(l, 3);
        bool jump        = lua_toboolean(l, 3) ? true : false;
        bool ret = player->loop(start, len, jump, 255, false);
        lua_pushboolean(l, ret ? 1 : 0);
        return 1;
    }
    return 0;
}

/**
 * @brief 设置拍速
 * tempo 1.0 表示元素，masterTempo true 变速不变调，false 变调
 */
static int player_set_tempo(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 3);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number)
    // check argv 3 is boolean
    CHECK_ARG_IS_STH(l, 3, boolean)

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number tempo = lua_tonumber(l, 2);
        bool mastertempo = lua_toboolean(l, 3) ? true : false;
        player->setTempo(tempo, mastertempo);
    }

    return 0;
}

static int player_set_volume(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 3);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number)

    if (p) {
        lua_Number volume = lua_tonumber(l, 2);
        superpowered_player_set_volume(p->player, volume);
    }

    return 0;
}

static int player_set_exit_loop(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 3);

    // check argv 2 is boolean
    CHECK_ARG_IS_STH(l, 2, boolean)

    if (p) {
        int exit = lua_toboolean(l, 2);
        superpowered_player_set_exit_loop(p->player, exit);
    }

    return 0;
}

static int player_set_pause_when_eof(lua_State* l) {
    LuaPlayer* p = check_arg1_is_lua_player(l, __func__, 3);

    // check argv 2 is boolean
    CHECK_ARG_IS_STH(l, 2, boolean)

    if (p) {
        int pause = lua_toboolean(l, 2);
        superpowered_player_set_pause_when_eof(p->player, pause);
    }

    return 0;
}

int superpowered_register_player_module(lua_State* l) {
    // create a table event callback index
    lua_pushstring(l, SUPERPOWERED_PLAYER_CALLBACKS);
    lua_newtable(l);
    lua_rawset(l, LUA_REGISTRYINDEX);

    // reg all functions
    const luaL_Reg reg[] = {{"create", player_create},
                            {"release", player_release},
                            // readonly
                            {"duration", player_duration},
                            {"position", player_position},
                            {"tempo", player_tempo},
                            {"playing", player_playing},
                            {"looping", player_looping},
                            {"volume", player_volume},
                            // ctrl
                            {"open", player_open},
                            {"play", player_play},
                            {"pause", player_pause},
                            {"seek", player_seek},
                            {"loop", player_loop},
                            {"set_tempo", player_set_tempo},
                            {"set_volume", player_set_volume},
                            {"set_exit_loop", player_set_exit_loop},
                            {"set_pause_when_eof", player_set_pause_when_eof},
                            {NULL, NULL}};

    lua_newtable(l);
    luaL_setfuncs(l, reg, 0);

    return 1;
}
