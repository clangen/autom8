#pragma once

#include <stdio.h>

namespace autom8 {
    typedef void (*on_message_received_func)(const char** argv, int argc);

    struct x10_device_controller {
        typedef bool (*init_func)(void);
        init_func init;

        typedef void (*deinit_func)(void);
        deinit_func deinit;

        typedef void (*set_message_received_callback_func)(on_message_received_func);
        set_message_received_callback_func set_message_received_callback;

        typedef bool (*send_device_message_func)(const char* message_type, const char* message_params);
        send_device_message_func send_device_message;

        typedef bool (*get_device_status_func)(const char* device_address);
        get_device_status_func get_device_status;
    };
}
