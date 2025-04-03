#ifndef PUBFUNCLIB_H
#define PUBFUNCLIB_H
#include <string>

namespace PubFunc
{
	/* 将输入的十进制转为十六进制 char字符 */
	unsigned char ToHex(unsigned char Input);
	/* 将输入的十六进制转为十进制 char字符 */
	unsigned char FromHex(unsigned char Hex);
	/* URL 转码 */
	std::string UrlEncode(const std::string& str);
	/* URL 解码 */
	std::string UrlDecode(const std::string& str);
}
#endif // PUBFUNCLIB_H
