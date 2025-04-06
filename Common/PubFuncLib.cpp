#include "PubFuncLib.h"
#include <cassert>
unsigned char PubFunc::ToHex(unsigned char Input)
{
	/**
	 * 10���ڵ����� �ַ� '0'~'9' ��ʾ
	 * 10���ϵ����� �ַ� 'A'~'F' ��ʾ
	 */

	return Input > 9 ? Input + 'A' : Input + '0';
}

unsigned char PubFunc::FromHex(unsigned char Hex)
{
	unsigned char y;
	if (Hex >= 'A' && Hex <= 'Z') y = Hex - 'A' + 10; //����9������ת�����ַ�
	else if (Hex >= 'a' && Hex <= 'z') y = Hex - 'a' + 10; //����Сд
	else if (Hex >= '0' && Hex <= '9') y = Hex - '0'; //10����ת �����ַ�
	else assert(0);
	return y;
}

std::string PubFunc::UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//�ж��Ƿ�������ֺ���ĸ����
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //Ϊ���ַ�
			strTemp += "+";
		else
		{
			//�����ַ���Ҫ��ǰ��%���Ҹ���λ�͵���λ�ֱ�תΪ16����
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
		//��ԭ+Ϊ��
		if (str[i] == '+') strTemp += ' ';
		//����%������������ַ���16����תΪchar��ƴ��
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
