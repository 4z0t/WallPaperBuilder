#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>


namespace Lua
{

	inline void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
	{
		lua_pushcfunction(l, func);
		lua_setglobal(l, name);
	}

	template<typename T>
	inline void _PushValue(lua_State* l, T arg);


	template<>
	inline void _PushValue(lua_State* l, lua_Integer arg)
	{
		lua_pushinteger(l, arg);
	}

	template<>
	inline void _PushValue(lua_State* l, int arg)
	{
		_PushValue<lua_Integer>(l, static_cast<lua_Integer>(arg));
	}

	template<>
	inline void _PushValue(lua_State* l, lua_Number arg)
	{
		lua_pushnumber(l, arg);
	}

	template<>
	inline void _PushValue(lua_State* l, const char* arg)
	{
		lua_pushstring(l, arg);
	}

	template<>
	inline void _PushValue(lua_State* l, std::nullptr_t arg)
	{
		lua_pushnil(l);
	}

	template<size_t N>
	size_t _PushArgs(lua_State* l)
	{
		return 0;
	}

	template<size_t N, typename T>
	size_t _PushArgs(lua_State* l, const T& arg);
	template<size_t N, typename T, typename ...Ts>
	size_t _PushArgs(lua_State* l, const T& arg, const Ts&... args);

	template<size_t N, typename T, typename ...Ts>
	size_t _PushArgs(lua_State* l, const T& arg, const Ts&... args)
	{
		_PushValue<T>(l, arg);
		return _PushArgs<N + 1, Ts...>(l, args...);
	}

	template<size_t N, typename T>
	size_t _PushArgs(lua_State* l, const T& arg)
	{
		_PushValue<T>(l, arg);
		return N + 1;
	}

	template<typename ...Ts>
	void CallFunction(lua_State* l, const char* name, const Ts&... args)
	{
		lua_getglobal(l, name);
		size_t n = _PushArgs<0, Ts...>(l, args...);
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
	Uint8 GetArg(lua_State* l, size_t Index)
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




	template<size_t Index, typename TResult>
	inline size_t _PushResult(lua_State* l, TResult& result)
	{
		return Index;
	}


	template<size_t Index, typename TResult, typename T, typename ...Ts>
	inline size_t _PushResult(lua_State* l, TResult& result)
	{
		_PushValue<T>(l, std::get<Index>(result));
		return _PushResult<Index + 1, TResult, Ts...>(l, result);
	}


	template<typename T>
	inline void _PushResult(lua_State* l, T result);

	template<typename T>
	inline size_t PushResult(lua_State* l, T result)
	{
		_PushResult<T>(l, result);
		return 1;
	}


	template<typename ...Ts>
	inline size_t PushResult(lua_State* l, std::tuple<Ts...>& result)
	{
		return _PushResult<0, std::tuple<Ts...>, Ts...>(l, result);
	}

	template<>
	inline  void _PushResult(lua_State* l, int result)
	{
		lua_pushinteger(l, result);
	}

	template<>
	inline void _PushResult(lua_State* l, double result)
	{
		lua_pushnumber(l, result);
	}

	template<>
	inline void _PushResult(lua_State* l, float result)
	{
		_PushResult<double>(l, result);
	}


	template<>
	inline void _PushResult(lua_State* l, const char* result)
	{
		lua_pushstring(l, result);
	}

	template<typename FnClass, typename ...TArgs>
	struct FunctionWrapper
	{
		static_assert(std::is_invocable<FnClass, TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
		using TReturn = typename std::invoke_result<FnClass, TArgs...>::type;
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
				FunctionWrapper::CallHelper(args, Indexes{});
				return 0;
			}
			else
			{
				TReturn result = FunctionWrapper::CallHelper(args, Indexes{});
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is>
		static TReturn CallHelper(ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			return FnClass{}(std::get<Is>(args)...);
		}
	};


	template<typename Fn, Fn fn, typename ...TArgs>
	struct CFunctionWrapper
	{
		static_assert(std::is_invocable<decltype(fn), TArgs...>::value, "Given function can't be called with such arguments!");
		using TReturn = typename std::invoke_result<decltype(fn), TArgs...>::type;
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
				CFunctionWrapper::CallHelper(args, Indexes{});
				return 0;
			}
			else
			{
				TReturn result = CFunctionWrapper::CallHelper(args, Indexes{});
				size_t n_results = PushResult(l, result);
				return n_results;
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

}