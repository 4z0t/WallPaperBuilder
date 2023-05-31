
#include <iostream>
#include <lua.hpp>



void DrawRect(int x, int y, int w, int h)
{
	std::cout << "drawing rect " << x << " " <<
		y << " " <<
		w << " " <<
		h << std::endl;
}

int Lua_DrawRect(lua_State* l)
{
	double x = luaL_checknumber(l, 1);
	double y = luaL_checknumber(l, 2);
	double w = luaL_checknumber(l, 3);
	double h = luaL_checknumber(l, 4);
	DrawRect(x, y, w, h);
	return 0;
}


int main(int argc, char** argv)
{
	using namespace std;
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, Lua_DrawRect);
	lua_setglobal(L, "DrawRect");

	if (luaL_dofile(L, "main.lua"))
	{
		cout << "error" << endl;
	}

	lua_getglobal(L, "Main");
	lua_call(L, 0, 1);

	lua_close(L);

	return 0;
}