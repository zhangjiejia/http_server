#include "lua.h"

lua_State* create_lua_context() {
    lua_State* L = luaL_newstate();
    if(!L) return NULL;
    luaL_openlibs(L);
    if(luaL_dofile(L, LUA_SCRIPT) != 0) return NULL;
    return L;
}

void close_lua_context(lua_State* context) {
    lua_close(context);
}

const char* handle_by_lua(lua_State* context, char* request) {
    lua_getglobal(context, LUA_HANDLER_NAME);
    
    if(!lua_isfunction(context, -1)) return NULL;
    
    lua_pushstring(context, request);
    lua_call(context, 1, 1);
    const char* result = lua_tostring(context, -1);
    lua_pop(context, 1);
    
    return result;
}