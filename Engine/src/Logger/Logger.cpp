#include "Logger.h"

std::vector<LogEntry> Logger::messages;

void Logger::Log(const std::string& message) {
    // TODO: Print on the console the message:
    // LOG: [ 12/Oct/2020 09:13:17 ] - Here goes the message ...
    // This should be displayed in green.
    time_t rawtime;
    struct tm* t = new tm;
    char buffer[80];
    time(&rawtime);
    localtime_s(t, &rawtime);
    strftime(buffer, 80, "%d-%b-%Y %H:%M:%S", t);
    std::cout << "\033[32m" << "LOG | " << buffer << " - " << message << "\033[0m" << std::endl;

    LogEntry log_entry;
    log_entry.type = LOG_INFO;
    log_entry.message = "LOG | " + static_cast<std::string>(buffer) + " - " + message;
    messages.push_back(log_entry);
}

void Logger::Err(const std::string& message) {
    // TODO: Print on the console the message:
     // ERR: [ 12/Oct/2020 09:13:17 ] - Here goes the message ...
     // This should be displayed in red.
    time_t rawtime;
    struct tm* t = new tm;
    char buffer[80];
    time(&rawtime);
    localtime_s(t, &rawtime);
    strftime(buffer, 80, "%d-%b-%Y %H:%M:%S", t);
    std::cout << "\033[31m" << "ERR | " << buffer << " - " << message << "\033[0m" << std::endl;

    LogEntry log_entry;
    log_entry.type = LOG_ERROR;
    log_entry.message = "ERR | " + static_cast<std::string>(buffer) + " - " + message;
    messages.push_back(log_entry);
}
