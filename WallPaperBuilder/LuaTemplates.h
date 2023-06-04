#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>

template<typename ...Ts>
void Lua_CallFunction(lua_State* l, const char* name, Ts&&... args)
{
	size_t n = _Lua_PushArgs<Ts..., 0>(l, std::forward<Ts>(arg)...);
	lua_call(l, n, 0);
}


template<typename T>
void _LuaPushValue(lua_State* l, T&& arg)
{
	std::cout << "Unknown type " << typeid(T).name() << std::endl;
	lua_pushnil(l);
}


template<>
void _LuaPushValue(lua_State* l, lua_Integer&& arg)
{
	lua_pushinteger(l, arg);
}

template<>
void _LuaPushValue(lua_State* l, lua_Number&& arg)
{
	lua_pushnumber(l, arg);
}

template<>
void _LuaPushValue(lua_State* l, const char*&& arg)
{
	lua_pushstring(l, arg);
}

template<typename T, typename ...Ts, size_t N>
size_t _Lua_PushArgs(lua_State* l, T&& arg, Ts&&... args)
{
	_LuaPushValue<T>(l, std::forward<T>(arg));
	return _Lua_PushArgs<Ts..., N+1>(l, std::forward<Ts>(args)...);
}

template<typename T, size_t N>
size_t _Lua_PushArgs(lua_State* l, T&& arg)
{
	_LuaPushValue<T>(l, std::forward<T>(arg));
	return N;
}

