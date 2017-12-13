#ifndef LUA_SUPERPOWERED_PLAYER_H_0HA5RJN4
#define LUA_SUPERPOWERED_PLAYER_H_0HA5RJN4

#include "lua_superpowered.h"

#ifdef __cplusplus
extern "C" {
#endif

SUPERPOWERED_EXPORT int superpowered_register_player_module(lua_State* l);

typedef void (*superpower_player_event_callback)(void* udata, int event, void* value);
/**
 * @brief Should init the player object in diff platform then return it
 */
SUPERPOWERED_EXPORT void* superpowered_player_create(superpower_player_event_callback callback, void* udata);

/**
 * @brief Should release the player object in diff platform
 */
SUPERPOWERED_EXPORT void superpowered_player_release(void* player);

/**
 * @brief return SuperpoweredAdvancedAudioPlayer object
 */
SUPERPOWERED_EXPORT void* superpowered_player_get(void* player);

/**
 * @brief get volume
 */
SUPERPOWERED_EXPORT float superpowered_player_volume(void* player);

/**
 * @brief set volume
 */
SUPERPOWERED_EXPORT void superpowered_player_set_volume(void* player, float volume);

/**
 * @param exit 0 doesn't exit loop when loop end otherwise exit
 */
SUPERPOWERED_EXPORT void superpowered_player_set_exit_loop(void* player, int exit);

/**
 * @param pause 1 pause player when event eof happen
 */
SUPERPOWERED_EXPORT void superpowered_player_set_pause_when_eof(void* player, int pause);

#ifdef __cplusplus
} /* end extern C */
#endif


#endif /* end of include guard: LUA_SUPERPOWERED_PLAYER_H_0HA5RJN4 */
