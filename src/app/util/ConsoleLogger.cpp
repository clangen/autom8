#include "ConsoleLogger.h"
#include <cursespp/SimpleScrollAdapter.h>
#include <app/util/Message.h>
#include <f8n/str/util.h>

#include <time.h>

using namespace f8n;
using namespace f8n::runtime;
using namespace autom8::app;
using namespace cursespp;

static const int MESSAGE_LOG = autom8::app::message::CreateType();

static std::string timestamp() {
    time_t rawtime = { 0 };
    struct tm * timeinfo;
    char buffer [64];
    time(&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer, sizeof(buffer), "%T", timeinfo);
    return std::string(buffer);
}

struct LogMessage: public message::BaseMessage {
    std::string logValue;

    LogMessage(IMessageTarget* target, const std::string& value)
    : BaseMessage(target, MESSAGE_LOG, 0, 0) {
        this->logValue = value;
    }
};

ConsoleLogger::ConsoleLogger(IMessageQueue& messageQueue)
: messageQueue(messageQueue) {
    this->adapter = std::make_shared<SimpleScrollAdapter>();
    this->adapter->SetSelectable(true);
}

void ConsoleLogger::verbose(const std::string& tag, const std::string& string) {
    this->FormatAndDispatch(tag, "v", string);
}

void ConsoleLogger::info(const std::string& tag, const std::string& string) {
    this->FormatAndDispatch(tag, "i", string);
}

void ConsoleLogger::warning(const std::string& tag, const std::string& string) {
    this->FormatAndDispatch(tag, "w", string);
}

void ConsoleLogger::error(const std::string& tag, const std::string& string) {
    this->FormatAndDispatch(tag, "e", string);
}

void ConsoleLogger::FormatAndDispatch(
    const std::string& tag, const std::string& level, const std::string& str)
{
    const std::string formatted = str::format(
        "%s [%s] [%s] %s", timestamp().c_str(), level.c_str(), tag.c_str(), str.c_str());

    this->messageQueue.Post(std::make_shared<LogMessage>(this, formatted));
}

void ConsoleLogger::ProcessMessage(IMessage& message) {
    if (message.Type() == MESSAGE_LOG) {
        this->adapter->AddEntry(static_cast<LogMessage*>(&message)->logValue);
    }
}

ConsoleLogger::AdapterPtr ConsoleLogger::Adapter() {
    return this->adapter;
}
