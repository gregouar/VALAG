#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"

#include <sstream>

Parser::Parser()
{
    //ctor
}

Parser::~Parser()
{
    //dtor
}

bool Parser::isBool(const std::string &data)
{
    bool value = false;
    std::istringstream ss(data);
    ss>>value;
    return ss.eof();
}

bool Parser::isInt(const std::string &data)
{
    int value = false;
    std::istringstream ss(data);
    ss>>value;
    return ss.eof();
}

bool Parser::isFloat(const std::string &data)
{
    float value = false;
    std::istringstream ss(data);
    ss>>value;
    return ss.eof();
}

bool  Parser::parseBool(const std::string& data)
{
    bool value = false;
    std::istringstream ss(data);
    ss>>value;
    if(!ss.eof())
    {
        if(data == "true" || data == "1")
            return (true);
        else
            return (false);
    }
    return value;
}

int   Parser::parseInt(const std::string& data)
{
    int value = 0;
    std::istringstream ss(data);
    ss>>value;
    return value;
}

float Parser::parseFloat(const std::string& data)
{
    float value = 0;
    std::istringstream ss(data);
    ss>>value;
    return value;
}

std::string Parser::findFileDirectory(const std::string& filePath)
{
    std::size_t p = filePath.find_last_of("/\\");
    if(p != std::string::npos)
        return filePath.substr(0,p+1);

    Logger::error("Cannot find directory of "+filePath);
    return filePath;
}
