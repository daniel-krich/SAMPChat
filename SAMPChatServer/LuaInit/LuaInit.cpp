/*#include "./LuaInit.h"

template<typename T> T implicit_convert(T t) { return t; }

LuaManager::LuaManager(std::string script_name)
{
	this->state = luaL_newstate();
	luaL_openlibs(this->state);
	luabridge::getGlobalNamespace(this->state).addFunction("PrintGUI", PrintGUI);
	luabridge::getGlobalNamespace(this->state).beginClass<ConnectedClient>("ConnectedClient")
		.addData<TCPServer*>("ServerInstance", &ConnectedClient::ServerInstance)
		.addData<nlohmann::json>("Storage", &ConnectedClient::Storage)
		.addFunction("Send", &ConnectedClient::Send)
		.addFunction("Close", &ConnectedClient::Close)
		.addFunction("GetIP", &ConnectedClient::GetIP)
		.endClass();
	luabridge::getGlobalNamespace(this->state).beginClass<TCPServer>("TCPServer")
		.endClass();
	luabridge::getGlobalNamespace(this->state).beginClass<nlohmann::json>("json")
		.addFunction("set", &nlohmann::json::operator=)
		.endClass();
	luaL_loadfile(this->state, script_name.c_str());
	lua_pcall(this->state, 0, 0, 0);
}

LuaManager::~LuaManager()
{
	lua_close(this->state);
}

lua_State* LuaManager::GetLuaState()
{
	return this->state;
}*/