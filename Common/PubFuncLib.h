#ifndef PUBFUNCLIB_H
#define PUBFUNCLIB_H
#include <string>

namespace PubFunc
{
	/* �������ʮ����תΪʮ������ char�ַ� */
	unsigned char ToHex(unsigned char Input);
	/* �������ʮ������תΪʮ���� char�ַ� */
	unsigned char FromHex(unsigned char Hex);
	/* URL ת�� */
	std::string UrlEncode(const std::string& str);
	/* URL ���� */
	std::string UrlDecode(const std::string& str);
}
#endif // PUBFUNCLIB_H
