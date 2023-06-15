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
void Test(lua_State* l)
{
	using namespace Tests;
	Lua::ClassWrapper<TestClass>::Init(l, "TestClass", TestClass::_class);

	std::cout << TestClass::className << std::endl;
}