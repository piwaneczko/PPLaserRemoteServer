#pragma once
#include <mutex>
#include <fstream>
#include <string>

using namespace std;

class Logger
{
    ofstream file;
    mutex locker;
    Logger();
public:
    static Logger &GetInstance();
    void Log(const char* file, const char* func, int line);
    void Log(const char* file, const char* func, int line, const string & text);
};

#define LOG_TIME() Logger::GetInstance().Log(__FILE__, __FUNCTION__, __LINE__)
#define LOG(text) Logger::GetInstance().Log(__FILE__, __FUNCTION__, __LINE__, text)