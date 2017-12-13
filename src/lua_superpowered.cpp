/**
 * Desc: 用于 superpowered 的 lua 接口的绑定
 * Auth: 张宇飞
 * Date: 2017-12-11
 */

#include "lua_superpowered.h"
#include "lua_superpowered_player.h"

#ifndef SUPERPOWERED_MODNAME
#define SUPERPOWERED_MODNAME "superpowered"
#endif  // end SUPERPOWERED_MODNAME

#ifndef SUPERPOWERED_VERSION
#define SUPERPOWERED_VERSION "0.1"
#endif  // end SUPERPOWERED_VERSION

int luaopen_superpowered(lua_State* l) {
    lua_newtable(l);
    if (superpowered_register_player_module(l)) {
        lua_setfield(l, -2, "player");
    }

    // set module name and version
    lua_pushliteral(l, SUPERPOWERED_MODNAME);
    lua_setfield(l, -2, "_NAME");
    lua_pushliteral(l, SUPERPOWERED_VERSION);
    lua_setfield(l, -2, "_VERSION");

    return 1;
}
