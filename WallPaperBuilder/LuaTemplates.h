#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>



template<typename T>
void _LuaPushValue(lua_State* l, T arg);


template<>
void _LuaPushValue(lua_State* l, lua_Integer arg)
{
	lua_pushinteger(l, arg);
}

template<>
void _LuaPushValue(lua_State* l, lua_Number arg)
{
	lua_pushnumber(l, arg);
}

template<>
void _LuaPushValue(lua_State* l, const char* arg)
{
	lua_pushstring(l, arg);
}


template<size_t N>
size_t _Lua_PushArgs(lua_State* l)
{
	return 0;
}

template<size_t N, typename T, typename ...Ts>
size_t _Lua_PushArgs(lua_State* l, T&& arg, const Ts&... args)
{
	_LuaPushValue<T>(l, arg);
	return _Lua_PushArgs<Ts..., N + 1>(l, args...);
}

template<size_t N, typename T>
size_t _Lua_PushArgs(lua_State* l, const T& arg)
{
	_LuaPushValue<T>(l, arg);
	return N + 1;
}

template<typename ...Ts>
void Lua_CallFunction(lua_State* l, const char* name, const Ts&... args)
{
	lua_getglobal(l, name);
	size_t n = _Lua_PushArgs<0, Ts...>(l, args...);
	lua_call(l, n, 0);
}
