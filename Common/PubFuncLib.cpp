#include "PubFuncLib.h"
#include <cassert>
unsigned char PubFunc::ToHex(unsigned char Input)
{
	/**
	 * 10以内的数用 字符 '0'~'9' 表示
	 * 10以上的数用 字符 'A'~'F' 表示
	 */

	return Input > 9 ? Input + 'A' : Input + '0';
}

unsigned char PubFunc::FromHex(unsigned char Hex)
{
	unsigned char y;
	if (Hex >= 'A' && Hex <= 'Z') y = Hex - 'A' + 10; //大于9的数字转数字字符
	else if (Hex >= 'a' && Hex <= 'z') y = Hex - 'a' + 10; //适配小写
	else if (Hex >= '0' && Hex <= '9') y = Hex - '0'; //10以内转 数字字符
	else assert(0);
	return y;
}

std::string PubFunc::UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//判断是否仅有数字和字母构成
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //为空字符
			strTemp += "+";
		else
		{
			//其他字符需要提前加%并且高四位和低四位分别转为16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string PubFunc::UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//还原+为空
		if (str[i] == '+') strTemp += ' ';
		//遇到%将后面的两个字符从16进制转为char再拼接
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}
