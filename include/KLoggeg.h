#include "CLIHandler.h"

inline Logger info(const std::string& context) {
    return CLIHandler::logger(Logger::Level::Info, context);
}

inline Logger log(const std::string& context) {
    return CLIHandler::logger(Logger::Level::Log, context);
}

inline Logger warning(const std::string& context) {
    return CLIHandler::logger(Logger::Level::Warning, context);
}

inline Logger error(const std::string& context) {
    return CLIHandler::logger(Logger::Level::Error, context);
}

inline Logger fatal(const std::string& context) {
    return CLIHandler::logger(Logger::Level::Fatal, context);
}
