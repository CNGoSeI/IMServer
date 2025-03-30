#include "const.h"
#include <filesystem>
#include <iostream>
#include "ConfigMgr.h"


const boost::property_tree::ptree& GateConfig::GetConfigHelper()
{

	static boost::property_tree::ptree pt= PublicConfig::CreateConfig("config.ini");
	return pt;
}
