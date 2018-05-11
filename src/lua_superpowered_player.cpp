#include "lua_superpowered_player.h"
#include "Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "mylog.h"

#define SUPERPOWERED_PLAYER_CALLBACKS "superpowered_player_callbacks"
#define LOG_ID "superpowered_player"
#define SUPERPOWERED_PLAYER_METATABLE "superpowered.player"

#define GET_SPPLAYER_FROM_LUA_PLAYER(_spplayer_name_, _lua_player_) \
    SuperpoweredAdvancedAudioPlayer* _spplayer_name_ =              \
        static_cast<SuperpoweredAdvancedAudioPlayer*>(superpowered_player_get(_lua_player_->player))

#define CHECK_ARG_IS_STH(_lua_state_, _n_, _type_)                                                                   \
    do{ if (!lua_is##_type_(_lua_state_, _n_)) {                                                                     \
        LOGD("[%s] invalid argument when call %s, argv %d need a %s", LOG_ID, __func__, _n_, #_type_); \
        return 0;                                                                                                    \
    }} while(0)

#define CHECK_ARG_IS_STH_IGNIL(_lua_state_, _n_, _type_)                                                             \
    do{ if (!lua_isnoneornil(_lua_state_, _n_)) {                                                                    \
        CHECK_ARG_IS_STH(_lua_state_, _n_, _type_);                                                                  \
    }} while(0)

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source. */
static void luaL_setfuncs (lua_State *l, const luaL_Reg *reg, int nup)
{
    int i;

    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(l, -nup);
        lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);  /* remove upvalues */
}
#endif

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
    LOGD("%s:udata:%p, event:%d", __func__, udata, event);
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

static LuaPlayer* check_lua_player_argc(lua_State* l, const char* funcname, int needargc) {
    int argc = lua_gettop(l);
    if (argc < needargc) {
        LOGD("[%s] invalid argument when call %s, and it needs %d arguments", LOG_ID, funcname, needargc);
        return NULL;
    }

    return static_cast<LuaPlayer*>(lua_touserdata(l, 1));
}

static int player_create(lua_State* l) {
    // check arg
    int argc = lua_gettop(l);
    if (argc != 1 || !lua_isfunction(l, 1)) {
        LOGD("[%s] %s need a event callback argument", LOG_ID, __func__);
        return 0;
    }

    // create player
    LuaPlayer* p = create_lua_player(l);
    p->player = superpowered_player_create((superpower_player_event_callback)player_event_callback, p);
    lua_pushlightuserdata(l, p);

    // set meta table
    luaL_getmetatable(l, SUPERPOWERED_PLAYER_METATABLE);
    lua_setmetatable(l, -2);

    return 1;
}

static int player_release(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        superpowered_player_release(p->player);
        p->player = nullptr;
        release_lua_player(p);
    }

    return 0;
}

static int player_duration(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->durationMs);
        return 1;
    }
    return 0;
}

static int player_position(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->positionMs);
        return 1;
    }
    return 0;
}

static int player_tempo(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushnumber(l, (lua_Number)player->tempo);
        lua_pushboolean(l, player->masterTempo ? 1 : 0);
        return 2;
    }
    return 0;
}

static int player_playing(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushboolean(l, player->playing ? 1 : 0);
        return 1;
    }
    return 0;
}

static int player_looping(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_pushboolean(l, player->looping ? 1 : 0);
        return 1;
    }
    return 0;
}

static int player_volume(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);
    if (p) {
        lua_pushnumber(l, (lua_Number)superpowered_player_volume(p->player));
        return 1;
    }
    return 0;
}

static int player_open(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is string
    CHECK_ARG_IS_STH(l, 2, string);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        const char* path = lua_tostring(l, 2);
        superpowered_player_open(p->player, path);
    }
    return 0;
}

static int player_play(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);

    // check argv 2 is boolean
    CHECK_ARG_IS_STH_IGNIL(l, 2, boolean);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        bool synchronised = lua_isnoneornil(l, 2) ? false : lua_toboolean(l, 2);
        player->play(synchronised);
    }

    return 0;
}

static int player_pause(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 1);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        player->pause();
    }

    return 0;
}

static int player_seek(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number);
    // check argv 3 is boolean
    CHECK_ARG_IS_STH_IGNIL(l, 3, boolean);
    // check argv 4 is boolean
    CHECK_ARG_IS_STH_IGNIL(l, 4, boolean);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number pos = lua_tonumber(l, 2);
        bool stop      = lua_isnoneornil(l, 3) ? false : lua_toboolean(l, 3);
        bool syncStart = lua_isnoneornil(l, 4) ? true : lua_toboolean(l, 4);
        player->setPosition(pos, stop, syncStart, true);
    }

    return 0;
}

static int player_loop(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 3);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number);
    // check argv 3 is number
    CHECK_ARG_IS_STH(l, 3, number);
    // check argv 4 is boolean
    CHECK_ARG_IS_STH_IGNIL(l, 4, boolean);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number start = lua_tonumber(l, 2);
        lua_Number len   = lua_tonumber(l, 3);
        bool jump        = lua_isnoneornil(l, 4) ? true : lua_toboolean(l, 4);
        bool ret = player->loop(start, len, jump, 255, true);
        lua_pushboolean(l, ret ? 1 : 0);
        return 1;
    }
    return 0;
}

/**
 * @brief 设置拍速
 * tempo 1.0 表示元素，masterTempo true 变速不变调，false 变调 默认为 true
 */
static int player_set_tempo(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number);
    // check argv 3 is boolean
    CHECK_ARG_IS_STH_IGNIL(l, 3, boolean);

    if (p) {
        GET_SPPLAYER_FROM_LUA_PLAYER(player, p);
        lua_Number tempo = lua_tonumber(l, 2);
        bool mastertempo = lua_isnoneornil(l, 3) ? true : lua_toboolean(l, 3);
        player->setTempo(tempo, mastertempo);
    }

    return 0;
}

static int player_set_volume(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is number
    CHECK_ARG_IS_STH(l, 2, number);

    if (p) {
        lua_Number volume = lua_tonumber(l, 2);
        superpowered_player_set_volume(p->player, volume);
    }

    return 0;
}

static int player_set_exit_loop(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is boolean
    CHECK_ARG_IS_STH(l, 2, boolean);

    if (p) {
        int exit = lua_toboolean(l, 2);
        superpowered_player_set_exit_loop(p->player, exit);
    }

    return 0;
}

static int player_set_pause_when_eof(lua_State* l) {
    LuaPlayer* p = check_lua_player_argc(l, __func__, 2);

    // check argv 2 is boolean
    CHECK_ARG_IS_STH(l, 2, boolean);

    if (p) {
        int pause = lua_toboolean(l, 2);
        superpowered_player_set_pause_when_eof(p->player, pause);
    }

    return 0;
}

static void create_player_meta_table(lua_State* l) {
    const luaL_Reg reg[] = {{"release", player_release},
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
                            {"setTempo", player_set_tempo},
                            {"setVolume", player_set_volume},
                            {"setExitLoop", player_set_exit_loop},
                            {"setPauseWhenEOF", player_set_pause_when_eof},
                            {NULL, NULL}};

    luaL_newmetatable(l, SUPERPOWERED_PLAYER_METATABLE);
    lua_newtable(l);
    luaL_setfuncs(l, reg, 0);
    lua_setfield(l, -2, "__index");
}

int superpowered_register_player_module(lua_State* l) {
    // create a table event callback index
    lua_pushstring(l, SUPERPOWERED_PLAYER_CALLBACKS);
    lua_newtable(l);
    lua_rawset(l, LUA_REGISTRYINDEX);

    create_player_meta_table(l);

    // reg event attribute
    lua_newtable(l);
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess);
    lua_setfield(l, -2, "Event_LoadSuccess");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_LoadError);
    lua_setfield(l, -2, "Event_LoadError");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_HLSNetworkError);
    lua_setfield(l, -2, "Event_HLSNetworkError");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_ProgressiveDownloadError);
    lua_setfield(l, -2, "Event_ProgressiveDownloadError");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_EOF);
    lua_setfield(l, -2, "Event_EOF");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_JogParameter);
    lua_setfield(l, -2, "Event_JogParameter");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_DurationChanged);
    lua_setfield(l, -2, "Event_DurationChanged");
    lua_pushinteger(l, SuperpoweredAdvancedAudioPlayerEvent_LoopEnd);
    lua_setfield(l, -2, "Event_LoopEnd");

    lua_pushcfunction(l, player_create);
    lua_setfield(l, -2, "create");

    return 1;
}
