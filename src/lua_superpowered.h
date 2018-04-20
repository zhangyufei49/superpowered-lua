#ifndef LUA_SUPERPOWERED_H_L5GMYF01
#define LUA_SUPERPOWERED_H_L5GMYF01

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

#ifdef _MSC_VER
#define SUPERPOWERED_EXPORT __declspec(dllexport)
#else
#define SUPERPOWERED_EXPORT extern
#endif  // end _MSC_VER

// copy from lua 5.2
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

void luaL_setfuncs(lua_State* l, const luaL_Reg* reg, int nup) {
    int i;

    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++)
        for (i = 0; i < nup; i++)
            lua_pushvalue(l, -nup);
        lua_pushcclosure(l, reg->func, nup);
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);
}

#endif  // end LUA_VERSION_NUM

SUPERPOWERED_EXPORT int luaopen_superpowered(lua_State* l);

#ifdef __cplusplus
}  // end extern C
#endif

#endif /* end of include guard: LUA_SUPERPOWERED_H_L5GMYF01 */
