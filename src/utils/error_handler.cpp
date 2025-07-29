#include "error_handler.hpp"
#include "logger.hpp"
#include "console_output.hpp"
#include <iostream>
#include <sstream>

namespace AutoVibez {

// Static member initialization
ErrorHandler::ErrorCallback ErrorHandler::errorCallback_ = nullptr;
bool ErrorHandler::hasErrorCallback_ = false;

// Thread-local context
thread_local std::string ErrorContext::currentContext_ = "";

void ErrorHandler::setErrorCallback(ErrorCallback callback) {
    errorCallback_ = std::move(callback);
    hasErrorCallback_ = true;
}

void ErrorHandler::handleError(const Error& error, bool canRecover) {
    // Log the error
    Logger::getInstance().error("Error: %s", error.what());
    
    // Add context if available
    if (!ErrorContext::currentContext_.empty()) {
        Logger::getInstance().error("Context: %s", ErrorContext::currentContext_.c_str());
    }
    
    // Call custom error callback if set
    if (hasErrorCallback_ && errorCallback_) {
        try {
            errorCallback_(error);
        } catch (...) {
            // Don't let error callback exceptions propagate
            Logger::getInstance().error("Error callback failed");
        }
    }
    
    // Handle based on severity
    Severity severity = getSeverity(error);
    switch (severity) {
        case Severity::INFO:
            ConsoleOutput::output(ConsoleOutput::Type::INFO, "ℹ️  %s", error.what());
            break;
        case Severity::WARNING:
            ConsoleOutput::output(ConsoleOutput::Type::WARNING, "⚠️  %s", error.what());
            break;
        case Severity::ERROR:
            ConsoleOutput::output(ConsoleOutput::Type::ERROR, "❌ %s", error.what());
            break;
        case Severity::CRITICAL:
            ConsoleOutput::output(ConsoleOutput::Type::ERROR, "💥 %s", error.what());
            if (!canRecover) {
                throw error; // Re-throw critical errors
            }
            break;
    }
}

void ErrorHandler::handleSystemError(const std::system_error& error) {
    Logger::getInstance().error("System error: %s (code: %d)", 
                               error.what(), error.code().value());
    
    ConsoleOutput::output(ConsoleOutput::Type::ERROR, "🔧 System error: %s", error.what());
}

void ErrorHandler::handleUnknownException(const std::exception& error) {
    Logger::getInstance().error("Unknown exception: %s", error.what());
    
    ConsoleOutput::output(ConsoleOutput::Type::ERROR, "❓ Unknown error: %s", error.what());
}

bool ErrorHandler::isRecoverable(const Error& error) {
    // Most errors are recoverable, except for critical ones
    Severity severity = getSeverity(error);
    return severity != Severity::CRITICAL;
}

ErrorHandler::Severity ErrorHandler::getSeverity(const Error& error) {
    const std::string& message = error.getMessage();
    
    // Check for critical keywords
    if (message.find("fatal") != std::string::npos ||
        message.find("critical") != std::string::npos ||
        message.find("unrecoverable") != std::string::npos) {
        return Severity::CRITICAL;
    }
    
    // Check for warning keywords
    if (message.find("warning") != std::string::npos ||
        message.find("deprecated") != std::string::npos) {
        return Severity::WARNING;
    }
    
    // Check for info keywords
    if (message.find("info") != std::string::npos ||
        message.find("note") != std::string::npos) {
        return Severity::INFO;
    }
    
    // Default to ERROR
    return Severity::ERROR;
}

// ErrorContext implementation
ErrorContext::ErrorContext(const std::string& context) : context_(context) {
    // Push context to thread-local storage
    if (!currentContext_.empty()) {
        currentContext_ += " > ";
    }
    currentContext_ += context_;
}

ErrorContext::~ErrorContext() {
    // Pop context from thread-local storage
    size_t pos = currentContext_.rfind(" > ");
    if (pos != std::string::npos) {
        currentContext_ = currentContext_.substr(0, pos);
    } else {
        currentContext_.clear();
    }
}

void ErrorContext::addContext(const std::string& key, const std::string& value) {
    context_ += " [" + key + ": " + value + "]";
}

std::string ErrorContext::getContext() const {
    return context_;
}

} // namespace AutoVibez 