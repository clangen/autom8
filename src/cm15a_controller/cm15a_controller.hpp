#pragma once

//////////////

#import "progid:X10.ActiveHome"
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <string>
#include <vector>
#include <queue>

//////////////

#ifdef CM15A_CONTROLLER_EXPORTS
#define CM15A_CONTROLLER_API __declspec(dllexport)
#else
#define CM15A_CONTROLLER_API __declspec(dllimport)
#endif

//////////////

class cm15a_status_notifier;

class cm15a_controller {
public:
    enum error {
        error_none = 0,
        error_failed_create_active_home,
        error_failed_create_custom_notifier,
        error_failed_query_generic_notifier,
        error_failed_atl_advise
    };

    cm15a_controller();
    ~cm15a_controller();

    bool send_action(const std::string& type, const std::string& params);
    bool query_status(const std::string& device);

    error get_error() { return error_; }

private:
    DWORD advise_cookie_;
    ActiveHomeScriptLib::IActiveHome* active_home_;
    ActiveHomeScriptLib::_IActiveHomeEvents* active_home_events_;
    CComObject<cm15a_status_notifier>* status_notifier_;
    error error_;
};

//////////////

struct critical_section {
    critical_section(HANDLE mutex) {
        mutex_ = mutex;
        ::WaitForSingleObject(mutex_, INFINITE);
    }

    ~critical_section() {
        ::ReleaseMutex(mutex_);
    }

    HANDLE mutex_;
};

//////////////

typedef void (*on_message_received_func)(const char** argv, int argc);
void raise_callback(const char** params, unsigned count);

//////////////

struct cm15a_message {
    cm15a_message(const char* type, const char* params)
    : type_(type)
    , params_(params) {
    }

    std::string type_;
    std::string params_;
};

typedef std::queue<cm15a_message> message_queue;

//////////////
// EXPORTS !!!
//////////////

extern "C" {
    CM15A_CONTROLLER_API
    bool init();

    CM15A_CONTROLLER_API
    void deinit();

    CM15A_CONTROLLER_API
    void set_message_received_callback(void* callback);

    CM15A_CONTROLLER_API
    bool send_device_message(const char* message_type, const char* message_params);

    CM15A_CONTROLLER_API
    bool get_device_status(const char* device_address);
}

//////////////