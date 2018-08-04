#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <sstream>
#include <mutex>

#include "Singleton.h"

class Logger : public Singleton<Logger>
{
    public:
        friend class Singleton<Logger>;

        static void write(const std::ostringstream&);
        static void warning(const std::ostringstream&);
        static void error(const std::ostringstream&);
        static void fatalError(const std::ostringstream&);

        static void write(const std::string&);
        static void warning(const std::string&);
        static void error(const std::string&);
        static void fatalError(const std::string&);

        void enableConsoleWriting();
        void disableConsoleWriting();

    protected:
        Logger();
        virtual ~Logger();

        std::mutex m_loggerMutex;

    private:
        std::ofstream *m_fileStream;
        bool m_enableConsoleWriting;
        bool m_enableWarnings;

        static const char *DEFAULT_LOG_PATH;
};

#endif // LOGGER_H
