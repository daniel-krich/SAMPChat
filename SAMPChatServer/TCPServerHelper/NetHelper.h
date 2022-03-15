
void enable_keepalive(SOCKET sock);
int get_range(int i1, int i2);

namespace str
{

	struct Breaker
	{
		std::string Cmd;
		std::string Params;
	};

	Breaker breakOnSpace(std::string text);

	bool compareChar(char& c1, char& c2);

	bool caseInSensStringCompare(std::string& str1, std::string& str2);

	constexpr unsigned int hash(const char* str, int h = 0)
	{
		return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
	}

	template <typename... T> std::string format(const char* text, T const&... args)
	{
		std::string Formatted(text);
		int args_count = 0;
		size_t pos;
		using expander = int[];
		(void)expander {
			0,
			(void(([&] {
				std::stringstream temp;
				temp << args;
				while ((pos = Formatted.find("{" + std::to_string(args_count) + "}")) != std::string::npos) {
					Formatted.replace(pos,
						std::string("{" + std::to_string(args_count) + "}").size(),
						temp.str());
				}
				args_count++;
			})()), 0)...
		};
		return Formatted;
	}

	template <typename I> std::string NumToHex(I w, size_t hex_len = sizeof(I) + 2) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
			rc[i] = digits[(w >> j) & 0x0f];
		return rc;
	}

	DWORD HexToNum(std::string Hex);
}