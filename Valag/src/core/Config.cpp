#include "Valag/core/Config.h"

#include <iostream>
#include <fstream>
#include "Valag/utils/Logger.h"
#include "Valag/utils/Parser.h"

namespace vlg{

Config::Config()
{
    //ctor
}

Config::~Config()
{
    //dtor
}


bool Config::load(const std::string& filePath)
{
    std::ifstream file;
    file.open(filePath.c_str(),std::ifstream::in);

    if(!file.is_open())
    {
        Logger::error("Can't open config file: "+filePath);
        return (false);
    }

    file.close();
    return (true);
}


bool Config::getBool(const std::string& s, const std::string& n, const std::string& d)
{
    return Parser::parseBool(Config::getString(s,n,d));
}

int Config::getInt(const std::string& section, const std::string& name, const std::string& dflt)
{
    const std::string &data = Config::getString(section,name,dflt);
    if(Parser::isInt(data))
        return Parser::parseInt(data);
    else
        return Parser::parseInt(dflt);
}

float Config::getFloat(const std::string& s, const std::string& n, const std::string& d)
{
    return Parser::parseFloat(Config::getString(s,n,d));
}

const std::string &Config::getString(const std::string& s, const std::string& n, const std::string& d)
{
    std::map<std::string, ConfigSection>::iterator  section_it;
    section_it = Config::instance()->m_sections.find(s);

    if(section_it != Config::instance()->m_sections.end())
    {
        ConfigSection::iterator option_it = section_it->second.find(n);

        if(option_it != section_it->second.end())
            return option_it->second;
    }

    return d;
}



}
