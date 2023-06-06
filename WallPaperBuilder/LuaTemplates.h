#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>


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

//template<class>
//class LuaFunc;

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

template<typename T>
T GetArg(lua_State* l, size_t Index);

template<>
float GetArg(lua_State* l, size_t Index)
{
	return luaL_checknumber(l, Index);
}

template<>
int GetArg(lua_State* l, size_t Index)
{
	return luaL_checkinteger(l, Index);
}

template<>
double GetArg(lua_State* l, size_t Index)
{
	return luaL_checknumber(l, Index);
}

template<>
const char* GetArg(lua_State* l, size_t Index)
{
	return luaL_checkstring(l, Index);
}



template<size_t N, typename TArgsTuple>
constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
{
	return N;
}


template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
{
	std::get<N>(args) = GetArg<TArg>(l, N + 1);
	return GetArgs<N + 1, TArgsTuple, TArgs...>(l, args);
}

template<typename T>
inline void PushResult(lua_State* l, T result);

template<>
inline  void PushResult(lua_State* l, int result)
{
	lua_pushinteger(l, result);
}

template<>
inline void PushResult(lua_State* l, double result)
{
	lua_pushnumber(l, result);
}

template<>
inline void PushResult(lua_State* l, float result)
{
	PushResult<double>(l, result);
}


template<>
inline void PushResult(lua_State* l, const char* result)
{
	lua_pushstring(l, result);
}

template<typename FnClass, typename ...TArgs>
struct Lua_FunctionWrapper
{
	static_assert(std::is_invocable<decltype(FnClass::Call), TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
	using TReturn = typename std::invoke_result<decltype(FnClass::Call), TArgs...>::type;
	using ArgsTupleT = std::tuple<TArgs...>;
	using Indexes = std::index_sequence_for<TArgs...>;

public:
	static int Function(lua_State* l)
	{
		using namespace std;
		ArgsTupleT args;
		GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
		
		if constexpr (std::is_void<TReturn>::value)
		{
			Lua_FunctionWrapper::CallHelper(args, Indexes{});
			return 0;
		}
		else
		{
			TReturn result = Lua_FunctionWrapper::CallHelper(args, Indexes{});
			PushResult<TReturn>(l, result);
			return 1;
		}
	}
private:
	template <std::size_t ... Is>
	static TReturn CallHelper(ArgsTupleT& args, std::index_sequence<Is...> const)
	{
		return FnClass::Call(std::get<Is>(args)...);
	}
};


template<typename Fn, Fn fn, typename ...TArgs>
struct _Lua_FunctionWrapper
{
	static_assert(std::is_invocable<decltype(fn), TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
	using TReturn = typename std::invoke_result<decltype(fn), TArgs...>::type;
	using ArgsTupleT = std::tuple<TArgs...>;

public:
	static int Function(lua_State* l)
	{
		using namespace std;
		ArgsTupleT args;
		GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
		if constexpr (std::is_void<TReturn>::value)
		{
			_Lua_FunctionWrapper::CallHelper(args, std::index_sequence_for<TArgs...>{});
			return 0;
		}
		else
		{
			TReturn result = _Lua_FunctionWrapper::CallHelper(args, std::index_sequence_for<TArgs...>{});
			PushResult<TReturn>(l, result);
			return 1;
		}
	}
private:
	template <std::size_t ... Is>
	static TReturn CallHelper(ArgsTupleT& args, std::index_sequence<Is...> const)
	{
		return fn(std::get<Is>(args)...);
	}
};

#define functype(f) decltype(f), f