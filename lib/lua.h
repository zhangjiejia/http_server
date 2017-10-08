#ifndef		__LUA_H__
#define		__LUA_H__

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define     LUA_SCRIPT              "script.lua"
#define     LUA_HANDLER_NAME        "handel"

lua_State* create_lua_context();

const char* handle_by_lua(lua_State* context, char* request);

void close_lua_context(lua_State* context);

#endif