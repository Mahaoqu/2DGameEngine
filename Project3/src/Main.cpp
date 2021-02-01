#include <iostream>
#include <sol/sol.hpp>

#include "Game/Game.h"

//int native_cpp_cube_function(int n) {
//	return n * n * n;
//}
//
//void TestLua() {
//	sol::state lua;
//	lua.open_libraries(sol::lib::base);
//
//	// This is how we expose and bind a native C++ function to  be used by the Lua script.
//	lua["cube"] = native_cpp_cube_function;
//
//	lua.script_file("./assets/scripts/myscript.lua");
//	int some_variable_inside_cpp = lua["some_variable"];
//	std::cout << "The value of some_variable in C++ is " << some_variable_inside_cpp << std::endl;
//
//	bool is_fullscreen = lua["config"]["fullscreen"];
//
//
//	sol::table config = lua["config"];
//
//	int width = config["resolution"]["width"];
//	int height = config["resolution"]["height"];
//	
//	std::cout << "We read the value fullscreen: " << is_fullscreen << ", width: " << width << ", height: " << height << std::endl;
//
//	// This is how we invoke a Lua function from inside C++.
//	sol::function function_factorial = lua["factorial"];
//	int function_result = function_factorial(5);
//	std::cout << "The Lua script calculated 5! as " << function_result << std::endl;
//}

int main(int argc, char* argv[]) {
	Game game;
	game.Initialize();
	game.Run();
	game.Destroy();
	return 0;
}