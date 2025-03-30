#include "ConfigMgr.h"
#include <iostream>
#include <filesystem>
#include <boost/property_tree/ini_parser.hpp>
#include "boost/property_tree/ptree.hpp"


using namespace PublicConfig;

boost::property_tree::ptree PublicConfig::CreateConfig(const std::string& FileName)
{
	// 获取当前工作目录  
	std::filesystem::path current_path = std::filesystem::current_path();
	// 构建config.ini文件的完整路径  
	std::filesystem::path config_path = current_path / FileName;
	std::cout << "Config path: " << config_path << std::endl;

	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini(config_path.string(), pt);

	return std::move(pt);
}
