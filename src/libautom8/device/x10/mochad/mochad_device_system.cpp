#include <autom8/device/x10/mochad/mochad_device_system.hpp>
#include <autom8/device/x10/mochad/mochad_controller.hpp>

#include <boost/thread.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <autom8/device/device_model.hpp>
#include <autom8/device/x10/x10_device_factory.hpp>
#include <autom8/util/utility.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <set>

/* example: 01/04 12:14:17 Tx PL HouseUnit: A4 */
#define MOCHAD_MESSAGE_REGEX "^\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2} (Tx|Rx) (PL|RFSEC) (HouseUnit: |House: )(.+)$"
#define LOWER(x) std::transform(x.begin(), x.end(), x.begin(), ::tolower)

using namespace autom8;

mochad_device_system::mochad_device_system() {
    factory_ = device_factory_ptr(new x10_device_factory(this));
    model_ = device_model_ptr(new device_model(factory_));

    model_->device_updated.connect(
        this, &mochad_device_system::on_device_updated);

    model_->device_removed.connect(
        this, &mochad_device_system::on_device_removed);

    controller_.message_received.connect(
        this, &mochad_device_system::on_message_received);
}

mochad_device_system::~mochad_device_system() {
}

bool update_x10_device(device_model& model, const std::string& address, const std::vector<std::string>& values) {
    device_ptr device = model.find_by_address(address);

    if (device) {
        x10_device* pdevice = dynamic_cast<x10_device*>(device.get());
        if (pdevice) {
            pdevice->on_controller_message(values);
            return true;
        }
    }

    return false;
}

void mochad_device_system::on_message_received(std::string message) {
    // std::cerr << "*** " << message << std::endl;

    boost::cmatch match;
    if (boost::regex_match(message.c_str(), match, boost::regex(MOCHAD_MESSAGE_REGEX))) {
        if (match.size() == 5) {
            std::string direction = match[1];
            std::string type = match[2];
            std::string first = match[3];
            std::string body = match[4];

            // std::cout << "*** direction " << direction << std::endl;
            // std::cout << "*** type " << type << std::endl;
            // std::cout << "*** body " << body << std::endl;

            if (type == "PL") {
                /* when devices are updated, mochad actually sends two messages. the
                first message is HouseUnit: XX, which specifies the address. we'll cache
                this as the last recv'd unit, and wait for the follow-up to tell us
                what changed */
                if (first == "HouseUnit: ") {
                    last_house_unit_ = body.substr(0, 2);
                    LOWER(last_house_unit_);
                    // std::cout << "*** last_house_unit " << last_house_unit_ << std::endl;
                }
                /* otherwise, body will contain "[HouseCode] Func: [EventData]" */
                else {
                    size_t pos = body.find("Func: ");
                    if (pos != std::string::npos) {
                        std::string func = body.substr(pos + 6); /* everything after "Func: " */
                        LOWER(func);

                        if (func.size() > 0 && last_house_unit_.size() > 0) {
                            /* ok, we have all the information required to process the message!
                            this part is a bit goofy and should be improved. basically, we need
                            to convert the mochad values into a list of values that the base
                            x10 subsystem can understand, which was coded up based on the win32
                            CM15A drivers */
                            std::vector<std::string> values;
                            values.push_back("pl");
                            values.push_back(last_house_unit_);
                            values.push_back(func); /* off|on */

                            update_x10_device(model(), last_house_unit_, values);
                        }
                    }

                    last_house_unit_ = "";
                }
            }
            else if (type == "RFSEC") {
                /* not currently supported */
            }
        }
    }
}

device_model& mochad_device_system::model() {
    return *model_.get();
}

void mochad_device_system::requery_device_status(const std::string& address) {
    controller_.requery(address);
}

bool mochad_device_system::send_device_message(command_type message_type, const char* message_params) {
    static int MOCHAD_MAX_LAMP_BRIGHTNESS = 31;
    static int X10_MAX_LAMP_BRIGHTNESS = 100;

    /* split the params into parts. depending on the message, we may
    need to futz with some of the params before we pass it along to
    the controller */
    std::string input(message_params);
    std::vector<std::string> parts;
    boost::split(parts, input, boost::is_any_of(" "));

    /* for bright/dim commands, x10_lamp assumes the range is from 0..100.
    however, mochad uses 0..31. normalize here before passing along*/
    if (parts.size() == 3 && (parts.at(1) == "bright" || parts.at(1) == "dim")) {
        try {
            int value = boost::lexical_cast<int>(parts.at(2));
            value = (value * MOCHAD_MAX_LAMP_BRIGHTNESS) / X10_MAX_LAMP_BRIGHTNESS;
            parts.at(2) = boost::lexical_cast<std::string>(value);
        }
        catch (boost::bad_lexical_cast) {
            /* nothing we can do... move on */
        }
    }

    /* reassemble message. as of right now, the base x10 device types will use
    messages in the following format: [TYPE] [ADDR] [CMD] [EXTRA].*/
    std::string message = (message_type == powerline_command) ? "pl " : "rf ";

    for (size_t i = 0; i < parts.size(); i++) {
        message += (" " + parts.at(i));
    }

    controller_.send(message);
    return true;
}

void mochad_device_system::on_device_removed(database_id id) {
    x10_device_factory* x10_factory = (x10_device_factory*) factory_.get();
    x10_factory->device_removed(id);
}

void mochad_device_system::on_device_updated(database_id id) {
    x10_device_factory* x10_factory = (x10_device_factory*) factory_.get();
    x10_factory->device_updated(id);
}
