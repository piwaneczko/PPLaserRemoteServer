#pragma once
#include <fstream>
#include <mutex>
#include <string>

using namespace std;

class Logger {
    ofstream file;
    mutex locker;
    Logger();

public:
    static Logger& getInstance();
    void log(const char* file, const char* func, int line);
    void log(const char* file, const char* func, int line, const string& text);
};

#define LOG_TIME() Logger::getInstance().Log(__FILE__, __FUNCTION__, __LINE__)
#define LOG(text) Logger::getInstance().Log(__FILE__, __FUNCTION__, __LINE__, text)
