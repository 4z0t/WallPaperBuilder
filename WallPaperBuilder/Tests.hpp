#pragma once


#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>
#include <tuple>
#include <vector>
#include <lua.hpp>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_timer.h>
#include "WallPaper.hpp"
#include "LuaTemplates.hpp"
#include "LuaState.hpp"




namespace Tests
{
	struct TestClass
	{
		static const char* className;


		static int Print(lua_State* l)
		{
			std::cout << "Hello!" << std::endl;

			return 0;
		}


		static int Create(lua_State* l)
		{
			size_t nbytes = sizeof(TestClass);
			TestClass* a = (TestClass*)lua_newuserdata(l, nbytes);

			luaL_getmetatable(l, TestClass::className);
			lua_setmetatable(l, -2);

			return 1;  /* new userdatum is already on the stack */
		}


		inline static const luaL_Reg meta[] = {
			{"print", TestClass::Print},
			{"aboba", TestClass::Print},
			{NULL, NULL}
		};

		inline static const luaL_Reg _class[] = {
			{"new", TestClass::Create},
			{NULL, NULL}
		};
	};

	const char* TestClass::className = typeid(TestClass).name();

}


class Callable
{
public:

	Callable(lua_State* l)
	{
		std::cout << "called with lua state in it" << std::endl;
	}

	int operator()(int a, int b)
	{
		return a * a + b;
	}

	int operator()(int a, int b, int c)
	{
		return a * a + b * c;
	}
};

template<typename T>
struct MakeArray
{
	std::vector<T> operator()(int n)
	{
		return std::vector<T>(n);
	}
};

void PrintClosureNumber(std::tuple<int, float>& upvalues)
{
	std::get<0>(upvalues)++;
	std::get<1>(upvalues) += 0.1;
	std::cout << "Value is " << std::get<0>(upvalues) << " " << std::get<1>(upvalues) << std::endl;
}

void Say(std::tuple<const char*>& upvalues)
{
	std::cout << "Value is " << std::get<0>(upvalues) << std::endl;
}


void Test(lua_State* l)
{
	using namespace Tests;
	Lua::ClassWrapper<TestClass>::Init(l, "TestClass", TestClass::_class);

	std::cout << TestClass::className << std::endl;

	Lua::RegisterFunction(l, "MakeArray", Lua::FunctionWrapper<MakeArray<int>, int>::Function);
	Lua::RegisterFunction(l, "DoubleInt", Lua::FunctionWrapper<Callable, int, int>::Function);
	Lua::RegisterFunction(l, "TripleInt", Lua::FunctionWrapper<Callable, int, int, int>::Function);
	Lua::RegisterClosure(l, "PrintInc", Lua::CClosureWrapper<PrintClosureNumber, std::tuple>::Function<int, float>, 7, 3.2f);
	Lua::RegisterClosure(l, "SayHello", Lua::CClosureWrapper<Say, std::tuple>::Function<const char*>, "Hello!");
	Lua::RegisterClosure(l, "SayBye", Lua::CClosureWrapper<Say, std::tuple>::Function<const char*>, "Bye!");


}