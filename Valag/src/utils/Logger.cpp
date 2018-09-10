#include "Valag/utils/Logger.h"

const char *Logger::DEFAULT_LOG_PATH = "log.txt";

Logger::Logger()
{
    m_enableConsoleWriting = true;
    m_enableWarnings = true;
    m_fileStream = new std::ofstream();
    m_fileStream->open(DEFAULT_LOG_PATH, std::ios::out | std::ios::trunc );
}

Logger::~Logger()
{
    m_fileStream->close();
    delete m_fileStream;
}

void Logger::write(const std::ostringstream& s)
{
    Logger::write(s.str());
}

void Logger::warning(const std::ostringstream& s)
{
    Logger::warning(s.str());
}

void Logger::error(const std::ostringstream& s)
{
    Logger::error(s.str());
}

void Logger::fatalError(const std::ostringstream& s)
{
    Logger::fatalError(s.str());
}

void Logger::write(const std::string& s)
{
    std::lock_guard<std::mutex> lock(Logger::instance()->m_loggerMutex);

    if(Logger::instance()->m_enableConsoleWriting)
        std::cout<<s<<std::endl;

    *Logger::instance()->m_fileStream<<s<<std::endl;
}

void Logger::warning(const std::string& s)
{
    if(Logger::instance()->m_enableWarnings)
        Logger::write("Warning : "+s);
}

void Logger::error(const std::string& s)
{
    Logger::write("ERROR: "+s);
}

void Logger::fatalError(const std::string& s)
{
    Logger::write("FATAL ERROR: "+s);
}

void Logger::enableConsoleWriting()
{
    Logger::instance()->m_enableConsoleWriting = true;
}

void Logger::disableConsoleWriting()
{
    Logger::instance()->m_enableConsoleWriting = false;
}

