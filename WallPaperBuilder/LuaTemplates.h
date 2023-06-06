#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>
#include <type_traits>



template<typename T>
inline void _LuaPushValue(lua_State* l, T arg);


template<>
inline void _LuaPushValue(lua_State* l, lua_Integer arg)
{
	lua_pushinteger(l, arg);
}

template<>
inline void _LuaPushValue(lua_State* l, lua_Number arg)
{
	lua_pushnumber(l, arg);
}

template<>
inline void _LuaPushValue(lua_State* l, const char* arg)
{
	lua_pushstring(l, arg);
}

template<>
inline void _LuaPushValue(lua_State* l, std::nullptr_t arg)
{
	lua_pushnil(l);
}

template<size_t N>
size_t _Lua_PushArgs(lua_State* l)
{
	return 0;
}

template<size_t N, typename T>
size_t _Lua_PushArgs(lua_State* l, const T& arg);
template<size_t N, typename T, typename ...Ts>
size_t _Lua_PushArgs(lua_State* l, const T& arg, const Ts&... args);

template<size_t N, typename T, typename ...Ts>
size_t _Lua_PushArgs(lua_State* l, const T& arg, const Ts&... args)
{
	_LuaPushValue<T>(l, arg);
	return _Lua_PushArgs<N + 1, Ts...>(l, args...);
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



//template<typename TReturn, typename ...TArgs>
// using Func = (TReturn (TArgs...);

template<class>
class LuaFunc;

//template<typename TReturn, typename ...TArgs>
//class LuaFunc<TReturn(TArgs...)>;
//


//template<typename TReturn, typename ...TArgs>
//class LuaFunc<int(lua_State*)>
//{
//	typedef TReturn _Func(TArgs...);
//public:
//	LuaFunc(_Func func) : _func(func)
//	{
//
//	}
//
//	int operator()(lua_State* l)
//	{
//
//	}
//private:
//	const _Func _func;
//};

template<size_t Index, typename T>
T GetArg(lua_State* l);

template<size_t Index>
float GetArg(lua_State* l)
{
	return luaL_checknumber(l, Index);
}

template<size_t Index>
int GetArg(lua_State* l)
{
	return luaL_checkinteger(l, Index);
}


template<size_t N, typename TArgsTuple>
void GetArgs(lua_State* l, TArgsTuple& args)
{
}


template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
void GetArgs(lua_State* l, TArgsTuple& args)
{
	std::get<N>(args) = GetArg<N + 1, TArg>(l);
	GetArgs<N + 1, TArgsTuple, TArgs...>(l, args);
}

template<typename T>
inline void PushResult(lua_State* l, T result);

template<>
inline  void PushResult(lua_State* l, int result)
{
	lua_pushinteger(l, result);
}

template<>
inline void PushResult(lua_State* l, float result)
{
	lua_pushnumber(l, result);
}

//template<typename TReturn, typename ...TArgs>
//constexpr lua_CFunction Lua_WrapFunction(static TReturn (*func)(TArgs...))
//{
//	using namespace std;
//	return  [func](lua_State* l) -> int
//	{
//		tuple<TArgs...> args;
//		GetArgs<0, TArgs ...>(l, args);
//		TReturn result = func(get<TArgs>(args)...);
//		PushResult<TReturn>(l, result);
//		return 1;
//	};
//}
//
//
//


template<typename FnClass, typename TReturn, typename ...TArgs>
struct Lua_FunctionWrapper;

template<typename FnClass, typename TReturn, typename ...TArgs>
struct Lua_FunctionWrapper
{
	//static_assert(std::is_same<decltype(FnClass::Call), TReturn(TArgs...)>::value);
	static int Function(lua_State* l)
	{
		using namespace std;
		using ArgsTupleT = tuple<TArgs...>;
		ArgsTupleT args;
		
		GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
		TReturn result = FnClass::Call(get<TArgs>(args)...);
		PushResult<TReturn>(l, result);
		return 1;
	}
};


//template<typename FnClass, typename TReturn, typename ...TArgs>
//constexpr lua_CFunction Lua_WrapFunction()
//{
//	using namespace std;
//	static_assert(is_same<decltype(FnClass::Call), TReturn(TArgs...)>::value);
//	return  [](lua_State* l) -> int
//	{
//		tuple<TArgs...> args;
//		GetArgs<0, TArgs ...>(l, args);
//		TReturn result = FnClass::Call(get<TArgs>(args)...);
//		PushResult<TReturn>(l, result);
//		return 1;
//	};
//}