#ifndef __C_AUTOM8_CONSTANTS_HPP__
#define __C_AUTOM8_CONSTANTS_HPP__

#define END_OF_MESSAGE 0
#define MESSAGE_URI_DELIMITER "\r\n"
#define MESSAGE_URI_DELIMITER_LENGTH 2
#define MESSAGE_URI_TYPE_REQUEST "request"
#define MESSAGE_URI_TYPE_RESPONSE "response"
#define MESSAGE_URI_REGEX_MATCH "^(autom8:\\/\\/)(request|response)/(\\w+)$"

namespace autom8 {
    enum device_type {
        device_type_unknown = -1,
        device_type_lamp = 0,
        device_type_appliance = 1,
        device_type_security_sensor = 2
    };

    enum command_type {
        powerline_command = 0,
        radio_frequency_command = 1
    };

    enum device_status {
        device_status_unknown = -1,
        device_status_off = 0,
        device_status_on = 1
    };

    typedef long long database_id;
}

#endif
