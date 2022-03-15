#include "../main.h"

MySQLConnector::MySQLConnector()
{
	try
	{
		sql::ConnectOptionsMap connection_properties;
		connection_properties["hostName"] = SQL_HOST;
		connection_properties["userName"] = SQL_USER;
		connection_properties["password"] = SQL_PASS;
		//
		this->driver = get_driver_instance();
		this->SqlConnection = this->driver->connect(connection_properties);
		this->SqlConnection->setSchema(SQL_DB);
		PrintGUI("[SQL] Connection to the MySQL database was successful.");
	}
	catch (const std::exception& ex)
	{
		PrintGUI(ex.what());
	}
}

MySQLConnector::~MySQLConnector() { }