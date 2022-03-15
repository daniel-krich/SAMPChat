/*#include "../main.h"
#include "../TCPServerHelper/TCPServerHelper.h"

class LuaManager
{
private:
	lua_State* state;
public:
	LuaManager(std::string script_name);
	~LuaManager();
	lua_State* GetLuaState();
	template<typename R, typename... T>R Call(std::string func, T... args)
	{
		luabridge::LuaRef funcCall = luabridge::getGlobal(this->state, func.c_str());
		if (funcCall.isFunction()) return funcCall(args...);
		else return NULL;
	}
};*/