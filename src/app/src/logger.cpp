#include "Logger.hpp"
#include <cassert>
#include <ctime>    // localtime
#include <iomanip>  // put_time
#include <sstream>  // stringstream

string GetFileName(string name) {
    size_t ndx = name.find_last_of('\\');
    if (ndx != name.npos) name = name.substr(ndx + 1);
    return name;
}
string GetDateTime() {
    tm tm;
    time_t t = time(nullptr);
    stringstream ss;
    localtime_s(&tm, &t);
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

Logger::Logger() : file("log.txt", ios::out | ios::app) {
    file << "=====   " + GetDateTime() + "   =====" << endl << flush;
}

Logger& Logger::GetInstance() {
    static Logger logger;
    return logger;
}

void Logger::Log(const char* file, const char* func, int line) {
    unique_lock<mutex> lock(locker);
    this->file << clock() << " - " << GetFileName(file) << " - " << func << "(" << line << ")" << endl << flush;
}

void Logger::Log(const char* file, const char* func, int line, const string& text) {
    unique_lock<mutex> lock(locker);
    this->file << clock() << " - " << GetFileName(file) << " - " << func << "(" << line << ") - " << text << endl << flush;
}
