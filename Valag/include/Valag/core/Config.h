#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <map>
#include "Valag/utils/singleton.h"

namespace vlg{

typedef std::map<std::string, std::string> ConfigSection;

class Config : public Singleton<Config>
{

    public:
        friend class Singleton<Config>;

        bool load(const std::string&);

        static bool                 getBool(const std::string& s, const std::string& n, const std::string& d=0);
        static int                  getInt(const std::string& s, const std::string& n, const std::string& d=0);
        static float                getFloat(const std::string& s, const std::string& n, const std::string& d=0);
        static const std::string&   getString(const std::string& s, const std::string& n, const std::string& d=0);

    protected:
        Config();
        virtual ~Config();

    private:
        std::map<std::string, ConfigSection> m_sections;
};

}

#endif // CONFIG_H
