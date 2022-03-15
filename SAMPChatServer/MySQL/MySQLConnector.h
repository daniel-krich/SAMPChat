

class MySQLConnector
{
private:
	sql::Driver* driver;
public:
	MySQLConnector();
	~MySQLConnector();
	ULONGLONG ReconnectInterval;
	sql::Connection* SqlConnection;
	template <typename... T> sql::ResultSet* get(std::string query, T... args)
	{
		try
		{
			unsigned int args_count = 1;
			sql::PreparedStatement* prep_stmt = this->SqlConnection->prepareStatement(query);
			prep_stmt->clearParameters();
			using expander = int[];
			(void)expander {
				0,
					(void(([&] {
					std::stringstream temp;
					temp << args;
					prep_stmt->setString(args_count, temp.str().c_str());
					args_count++;
				})()), 0)...
			};
			sql::ResultSet* res = prep_stmt->executeQuery();
			delete prep_stmt;
			return res;
		}
		catch (...)
		{
			return nullptr;
		}
		return nullptr;
	}

	template <typename... T> bool run(std::string query, T... args)
	{
		try
		{
			unsigned int args_count = 1;
			sql::PreparedStatement* prep_stmt = this->SqlConnection->prepareStatement(query);
			prep_stmt->clearParameters();
			using expander = int[];
			(void)expander {
				0,
					(void(([&] {
					std::stringstream temp;
					temp << args;
					prep_stmt->setString(args_count, temp.str().c_str());
					args_count++;
				})()), 0)...
			};
			prep_stmt->executeQuery();
			delete prep_stmt;
			return true;
		}
		catch (...)
		{
			return false;
		}
		return false;
	}
};
