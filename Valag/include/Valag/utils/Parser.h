#ifndef PARSER_H
#define PARSER_H

#include "Valag/utils/singleton.h"

///I should maybe switch to functions in a namespace

class Parser : public Singleton<Parser>
{
    public:
        friend class Singleton<Parser>;

        static bool isBool(const std::string&);
        static bool isInt(const std::string&);
        static bool isFloat(const std::string&);

        static bool  parseBool(const std::string&);
        static int   parseInt(const std::string&);
        static float parseFloat(const std::string&);

        static std::string findFileDirectory(const std::string&);

    protected:
        Parser();
        virtual ~Parser();

    private:
};

#endif // PARSER_H
