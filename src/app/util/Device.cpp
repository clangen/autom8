#include "Device.h"
#include <autom8/constants.h>
#include <autom8/message/request.hpp>

namespace autom8 { namespace app { namespace device {

    bool Toggle(autom8::client_ptr client, nlohmann::json& device) {
        std::string address = device.value("address", "");
        if (address.size()) {
            autom8::device_type type = device.value("type", autom8::device_type_unknown);
            if (type == autom8::device_type_lamp || type == autom8::device_type_appliance) {
                autom8::device_status status = device.value("status", autom8::device_status_unknown);
                autom8::device_status updated = (status == autom8::device_status_on)
                    ? autom8::device_status_off : autom8::device_status_on;

                nlohmann::json body = {
                    { "command", {
                        { "name", "set_status" },
                        { "address", address },
                        { "type", (int) autom8::powerline_command },
                        { "parameters", {
                            { "status", (int) updated }
                        }}
                    }}
                };

                client->send(request::create(
                    "autom8://request/send_device_command", body
                ));

                return true;
            }
        }
        return false;
    }

    autom8::device_type Type(const nlohmann::json& device) {
        return device.value("type", autom8::device_type_unknown);
    }

    int GetBrightness(const nlohmann::json& device) {
        return device
            .value("attributes", nlohmann::json::object())
            .value("brightness", 100);
    }

    bool SetBrightness(autom8::client_ptr client, const nlohmann::json& device, int brightness) {
        std::string address = device.value("address", "");
        if (address.size() && Type(device) == autom8::device_type_lamp) {
            nlohmann::json body = {
                { "command", {
                    { "name", "set_brightness" },
                    { "address", address },
                    { "type", (int) autom8::powerline_command },
                    { "parameters", {
                        { "brightness", (int) brightness }
                    }}
                }}
            };

            client->send(request::create(
                "autom8://request/send_device_command", body
            ));

            return true;
        }
        return false;
    }

}}}