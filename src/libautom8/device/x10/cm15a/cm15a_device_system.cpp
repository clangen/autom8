#include <autom8/device/x10/cm15a/cm15a_device_system.hpp>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <autom8/device/device_model.hpp>
#include <autom8/device/x10/x10_device_factory.hpp>
#include <autom8/util/utility.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <set>

using namespace autom8;

typedef std::set<cm15a_device_system*> instance_list;

/* magic constant we can use to get the path to the dll (libautom8.dll) */
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

/* we just let these leak; the cm15a_device_system instances are wrapped
 * in shared_ptrs(), which may be deleted after static variable destructors
 * have been called. */
static instance_list *instances_ = new instance_list();
static boost::mutex *instance_mutex_ = new boost::mutex();

void on_message_received(const char **argv, int argc) {
    boost::mutex::scoped_lock lock(*instance_mutex_);

    instance_list::iterator it = instances_->begin();
    while (it != instances_->end()) {
        (*it)->on_message_received(argv, argc);
        ++it;
    }
}

cm15a_device_system::cm15a_device_system()
: is_functional_(false) {
    /* add path to libautom8.dll to library path. this ensures that the
    cm15a_controller.dll (and other required dlls) can be loaded properly
    at runtime */
    wchar_t buffer[4096];
    ::GetModuleFileName((HMODULE)&__ImageBase, buffer, 4096);
    boost::filesystem::path module_path(buffer);
    ::SetDllDirectory(module_path.parent_path().wstring().c_str());

    // create the factory and model
    factory_ = device_factory_ptr(new x10_device_factory(this));
    model_ = device_model_ptr(new device_model(factory_));

    // listen to the model's signals
    model_->device_updated.connect(
        this, &cm15a_device_system::on_device_updated);
    //
    model_->device_removed.connect(
        this, &cm15a_device_system::on_device_removed);

    ::memset(&controller_, 0, sizeof(controller_));

    // the cm15a stuff has to live in a dll becuase it uses a single threaded COM
    // apartment, which means messages are delivered via standard windows message
    // pump. the server doesn't have a message pump, so it must run in a dll.
    dll_ = ::LoadLibrary(L"cm15a_controller.dll");
    if (dll_ != NULL) {
        typedef x10_device_controller c;
        controller_.init = (c::init_func) ::GetProcAddress(dll_, "init");
        controller_.deinit = (c::deinit_func) ::GetProcAddress(dll_, "deinit");
        controller_.set_message_received_callback = (c::set_message_received_callback_func) ::GetProcAddress(dll_, "set_message_received_callback");
        controller_.send_device_message = (c::send_device_message_func) ::GetProcAddress(dll_, "send_device_message");
        controller_.get_device_status   = (c::get_device_status_func) ::GetProcAddress(dll_, "get_device_status");
    }
    else {
        MessageBox(NULL, L"cm15a_controller.dll not found. this program will now crash.", L"autom8 server", MB_OK);
        throw std::exception();
    }

    is_functional_ = true;

    {
        boost::mutex::scoped_lock lock(*instance_mutex_);
        instances_->insert(this);

        if (instances_->size() == 1) {
            is_functional_ = controller_.init();
        }
    }

    if (is_functional_) {
        controller_.set_message_received_callback(::on_message_received);
    }
}

cm15a_device_system::~cm15a_device_system() {
    {
        boost::mutex::scoped_lock lock(*instance_mutex_);
        instances_->erase(instances_->find(this));

        if (instances_->size() == 0) {
            controller_.deinit();
        }
    }

    if (dll_ != NULL) {
        ::FreeLibrary(dll_);
        dll_ = NULL;
    }
}

void cm15a_device_system::on_message_received(const char **params, int count) {
    if (count > 1) {
        // device address is always the second param
        std::string address(params[1]);
        device_ptr device = model().find_by_address(address);
        if (device) {
            x10_device* pdevice = dynamic_cast<x10_device*>(device.get());
            if ( ! pdevice) {
                return;
            }

            std::vector<std::string> param_list;
            for (int i = 0; i < count; i++) {
                param_list.push_back(std::string(params[i]));
            }

            pdevice->on_controller_message(param_list);
        }
    }
}

device_model& cm15a_device_system::model() {
    return *model_.get();
}

void cm15a_device_system::requery_device_status(const std::string& address) {
    controller_.get_device_status(address.c_str());
}

bool cm15a_device_system::send_device_message(command_type message_type, const char* message_params) {
    return controller_.send_device_message(
        (message_type == powerline_command) ? "sendplc" : "sendrf", message_params);
}

void cm15a_device_system::on_device_removed(database_id id) {
    x10_device_factory* x10_factory = (x10_device_factory*) factory_.get();
    x10_factory->device_removed(id);
}

void cm15a_device_system::on_device_updated(database_id id) {
    x10_device_factory* x10_factory = (x10_device_factory*) factory_.get();
    x10_factory->device_updated(id);
}
