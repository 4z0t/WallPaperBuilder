#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


namespace Lua
{

	inline void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
	{
		lua_pushcfunction(l, func);
		lua_setglobal(l, name);
	}


	template<typename T>
	inline void _PushValue(lua_State* l, const T* arg);

	template<>
	inline void _PushValue(lua_State* l, const char* arg)
	{
		lua_pushstring(l, arg);
	}

	template<typename T>
	inline void _PushValue(lua_State* l, T arg);


	/*template<size_t N>
	inline void _PushValue(lua_State* l, const char arg[N])
	{
		lua_pushstring(l, arg);
	}*/

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
	inline void _PushValue(lua_State* l, float arg)
	{
		_PushValue<lua_Number>(l, static_cast<lua_Number> (arg));
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
		_PushValue(l, arg);
		return _PushArgs<N + 1, Ts...>(l, args...);
	}

	template<size_t N, typename T>
	size_t _PushArgs(lua_State* l, const T& arg)
	{
		_PushValue(l, arg);
		return N + 1;
	}

	template<typename ...Ts>
	void CallFunction(lua_State* l, const char* name, const Ts&... args)
	{
		lua_getglobal(l, name);
		size_t n = _PushArgs<0, Ts...>(l, args...);
		lua_call(l, n, 0);
	}

	template<typename ...Ts>
	bool CallFunctionProtected(lua_State* l, const char* name, const Ts&... args)
	{
		lua_getglobal(l, name);
		size_t n = _PushArgs<0, Ts...>(l, args...);
		return lua_pcall(l, n, 0, 0) == LUA_OK;
	}


	template<typename ...Ts>
	void RegisterClosure(lua_State* l, const char* name, lua_CFunction func, const Ts&... args)
	{
		size_t n = _PushArgs<0, Ts...>(l, args...);
		lua_pushcclosure(l, func, n);
		lua_setglobal(l, name);
	}

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

	template<size_t N, typename TArgsTuple>
	constexpr size_t GetUpvalue(lua_State* l, TArgsTuple& args)
	{
		return N;
	}

	template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
	constexpr size_t GetUpvalue(lua_State* l, TArgsTuple& args)
	{
		std::get<N>(args) = GetArg<TArg>(l, lua_upvalueindex(N + 1));
		return GetUpvalue<N + 1, TArgsTuple, TArgs...>(l, args);
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
	inline void _PushResult(lua_State* l, T result)
	{
		_PushValue<T>(l, result);
	}

	template<typename T>
	inline void _PushResult(lua_State* l, std::vector<T>& result)
	{
		lua_createtable(l, result.size(), 0);
		for (size_t i = 0; i < result.size(); i++) {
			_PushValue<T>(l, result[i]);
			lua_rawseti(l, -2, i + 1);
		}
	}

	template<typename T>
	inline size_t PushResult(lua_State* l, T result)
	{
		_PushResult(l, result);
		return 1;
	}


	template<typename ...Ts>
	inline size_t PushResult(lua_State* l, std::tuple<Ts...>& result)
	{
		return _PushResult<0, std::tuple<Ts...>, Ts...>(l, result);
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
				FunctionWrapper::CallHelper(l, args, Indexes{});
				return 0;
			}
			else
			{
				TReturn result = FunctionWrapper::CallHelper(l, args, Indexes{});
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is>
		static TReturn CallHelper(lua_State* l, ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			if constexpr (std::is_constructible<FnClass, lua_State*>::value)
			{
				return FnClass{ l }(std::get<Is>(args)...);
			}
			else
			{
				return FnClass{}(std::get<Is>(args)...);
			}
		}
	};


	template<auto fn, typename ...TArgs>
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

	template<size_t Index, typename TResult>
	constexpr size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
	{
		return Index;
	}

	template<size_t Index, typename TResult, typename T, typename ...Ts>
	inline size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
	{
		if constexpr (!std::is_pointer<T>::value)
		{
			_PushValue<T>(l, std::get<Index>(upvalues));
			lua_replace(l, lua_upvalueindex(Index + 1));
		}
		return _ReplaceUpvalue<Index + 1, TResult, Ts...>(l, upvalues);
	}

	template<typename ...CArgs>
	void ReplaceUpvalues(lua_State* l, std::tuple<CArgs...>& upvalues)
	{
		_ReplaceUpvalue<0, std::tuple<CArgs...>, CArgs...>(l, upvalues);
	}

	template<auto fn, template<typename ...CArgs> typename Upvalues, typename ...TArgs>
	struct CClosureWrapper
	{
		template<typename ...CArgs>
		using TReturn = typename std::invoke_result<decltype(fn), Upvalues<CArgs...>&, TArgs...>::type;
		using ArgsTupleT = std::tuple<TArgs...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		template<typename ...CArgs>
		static int Function(lua_State* l)
		{
			using namespace std;
			static_assert(std::is_invocable<decltype(fn), Upvalues<CArgs...>&, TArgs...>::value, "Given function can't be called with such arguments!");

			Upvalues<CArgs...> upvalues;
			GetUpvalue<0, Upvalues<CArgs...>, CArgs...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (std::is_void<TReturn<CArgs...>>::value)
			{
				CClosureWrapper::CallHelper(upvalues, args, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<CArgs...> result = CClosureWrapper::CallHelper(upvalues, args, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return n_results;
			}
		}
	private:
		template <std::size_t ... Is, typename ...CArgs>
		static TReturn<CArgs...> CallHelper(Upvalues<CArgs...>& upvalues, ArgsTupleT& args, std::index_sequence<Is...> const)
		{
			return fn(upvalues, std::get<Is>(args)...);
		}
	};




	/*void luaL_openlib(lua_State* L, const char* name, const luaL_Reg* reg, int nup)
	{
		lua_newtable(L);
		luaL_setfuncs(L, reg, nup);
		lua_setglobal(L, name);
	}*/

	constexpr size_t RegisterSize(const struct luaL_Reg* reg)
	{
		size_t s = 0;
		while ((reg + s)->name != NULL) s++;
		return s;
	}

	template<class TClass>
	class ClassWrapper
	{
		//static_assert(std::is_same<decltype(TClass::meta),const luaL_Reg[]>::value);
		static_assert(TClass::meta != nullptr, "Metatable required");
	public:
		static void Init(lua_State* l, const char* name, const struct luaL_Reg class_reg[])
		{
			//create metatable with class name as metatable name
			luaL_newmetatable(l, typeid(TClass).name());
			lua_pushstring(l, "__index");
			lua_pushvalue(l, -2);
			lua_settable(l, -3);
			luaL_setfuncs(l, TClass::meta, 0);

			//create lib
			lua_createtable(l, 0, RegisterSize(class_reg));
			luaL_setfuncs(l, class_reg, 0);
			lua_setglobal(l, name);
		}
	};



}