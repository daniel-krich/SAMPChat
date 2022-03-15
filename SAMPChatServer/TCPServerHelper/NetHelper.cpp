#include "../main.h"

void enable_keepalive(SOCKET sock)
{
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&yes), sizeof(int));
    int idle = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, reinterpret_cast<char*>(&idle), sizeof(int));
    int interval = 5;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, reinterpret_cast<char*>(&interval), sizeof(int));
    int maxpkt = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, reinterpret_cast<char*>(&maxpkt), sizeof(int));
}

int get_range(int i1, int i2)
{
    if (i1 > i2) return i1 - i2;
    else if (i1 < i2) return i2 - i1;
    else return 0;
}

str::Breaker str::breakOnSpace(std::string text)
{
    str::Breaker ret;
    for (size_t i = 0; i <= text.size(); i++)
    {
        if (text[i] == ' ')
        {
            if ((i + 1) <= text.size())
            {
                ret.Params = text.substr(i + 1);
            }
            ret.Cmd = text.substr(0, i);
            break;
        }
    }
    if (ret.Cmd.length() == 0) ret.Cmd = text;
    return ret;
}

bool str::compareChar(char& c1, char& c2)
{
    if (c1 == c2)
        return true;
    else if (std::toupper(c1) == std::toupper(c2))
        return true;
    return false;
}

bool str::caseInSensStringCompare(std::string& str1, std::string& str2)
{
    return ((str1.size() == str2.size()) &&
        std::equal(str1.begin(), str1.end(), str2.begin(), &str::compareChar));
}

DWORD str::HexToNum(std::string Hex)
{
    static const std::string digits = "0123456789ABCDEF";
    DWORD hexNum = 0;
    for (int i = 0; i < (int)Hex.size(); i++)
    {
        Hex[i] = std::toupper(Hex[i]);
        for (int hexRange = 0; hexRange < (int)digits.size(); hexRange++)
        {
            if (Hex[i] == digits[hexRange])
            {
                hexNum += (hexRange * (int)std::pow(16, ((int)Hex.size() - i - 1)));
            }
        }
    }
    return hexNum;
}