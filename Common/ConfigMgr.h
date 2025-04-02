#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace PublicConfig
{
	struct SectionInfo {
		SectionInfo() {}
		~SectionInfo() {
			_section_datas.clear();
		}

		SectionInfo(const SectionInfo& src) {
			_section_datas = src._section_datas;
		}

		SectionInfo& operator = (const SectionInfo& src) {
			if (&src == this) {
				return *this;
			}

			this->_section_datas = src._section_datas;
		}

		std::map<std::string, std::string> _section_datas;
		std::string  operator[](const std::string& key) {
			if (_section_datas.find(key) == _section_datas.end()) {
				return "";
			}
			// 这里可以添加一些边界检查  
			return _section_datas[key];
		}
	};

	boost::property_tree::ptree CreateConfig(const std::string& FileName);
}

namespace Mgr
{
	const boost::property_tree::ptree& GetConfigHelper();//获取配置读取
}

#endif // CONFIGMGR_H
