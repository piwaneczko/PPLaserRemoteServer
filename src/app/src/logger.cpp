#include "Logger.hpp"
#include <cassert>
#include <ctime>    // localtime
#include <iomanip>  // put_time
#include <sstream>  // stringstream

string getFileName(string name) {
    const auto ndx = name.find_last_of('\\');
    if (ndx != name.npos) name = name.substr(ndx + 1);
    return name;
}
string getDateTime() {
    tm tm;
    time_t t = time(nullptr);
    stringstream ss;
    localtime_s(&tm, &t);
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

Logger::Logger() : file("log.txt", ios::out | ios::app) {
    file << "=====   " + getDateTime() + "   =====" << endl << flush;
}

Logger& Logger::getInstance() {
    static Logger logger;
    return logger;
}

void Logger::log(const char* file, const char* func, int line) {
    lock_guard<mutex> lock(locker);
    this->file << clock() << " - " << getFileName(file) << " - " << func << "(" << line << ")" << endl << flush;
}

void Logger::log(const char* file, const char* func, int line, const string& text) {
    lock_guard<mutex> lock(locker);
    this->file << clock() << " - " << getFileName(file) << " - " << func << "(" << line << ") - " << text << endl << flush;
}
