#include "stdafx.h"
#include "cm15a_controller.hpp"

//////////////
// GLOBAL !!!!
//////////////

static CComModule _Module;
static HANDLE mutex_ = ::CreateMutex(NULL, FALSE, NULL);
static HANDLE quit_event_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
static HANDLE message_thread_ = NULL;
static HWND message_hwnd_ = NULL;
static DWORD reference_count_ = 0;
static cm15a_controller* cm15a_ = NULL;
static on_message_received_func message_received_callback_;
static message_queue queue_;
static bool message_loop_is_running_ = false;

//////////////
// COM !!!!!!!
//////////////

class ATL_NO_VTABLE cm15a_status_notifier :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<cm15a_status_notifier, &CLSID_NULL>,
    public ActiveHomeScriptLib::_IActiveHomeEvents {

public:
    cm15a_status_notifier() {
    }

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(cm15a_status_notifier)
        COM_INTERFACE_ENTRY(ActiveHomeScriptLib::_IActiveHomeEvents)
    END_COM_MAP()

    STDMETHOD(RecvAction) (
        VARIANT bszAction,
        VARIANT bszParm1,
        VARIANT bszParm2,
        VARIANT bszParm3,
        VARIANT bszParm4,
        VARIANT bszParm5,
        VARIANT bszReserved)
	{
        return handleRecv(bszAction, bszParm1, bszParm2,
			bszParm3, bszParm4, bszParm5, bszReserved);
    }

    STDMETHOD(raw_RecvAction)(
        VARIANT bszAction,
        VARIANT bszParm1,
        VARIANT bszParm2,
        VARIANT bszParm3,
        VARIANT bszParm4,
        VARIANT bszParm5,
        VARIANT bszReserved)
	{
        return handleRecv(bszAction, bszParm1, bszParm2,
			bszParm3, bszParm4, bszParm5, bszReserved);
    }

private:
	inline HRESULT handleRecv(
        VARIANT bszAction,
        VARIANT bszParm1,
        VARIANT bszParm2,
        VARIANT bszParm3,
        VARIANT bszParm4,
        VARIANT bszParm5,
        VARIANT bszReserved) {

        const char* params[6];

        std::string p0 = variantToString(bszAction); params[0] = p0.c_str();
        std::string p1 = variantToString(bszParm1); params[1] = p1.c_str();
        std::string p2 = variantToString(bszParm2); params[2] = p2.c_str();
        std::string p3 = variantToString(bszParm3); params[3] = p3.c_str();
        std::string p4 = variantToString(bszParm4); params[4] = p4.c_str();
        std::string p5 = variantToString(bszParm5); params[5] = p5.c_str();

        raise_callback(params, 6);

        return S_OK;
	}

    inline std::string variantToString(VARIANT& v) {
        switch (v.vt) {
        case VT_BSTR:
            return std::string(CW2A(v.bstrVal));

        case VT_I4:
            char buffer[33] = { 0 };
            _itoa_s(v.iVal, buffer, sizeof(buffer), 10);
            return std::string(buffer);
        }

        return "";
    }
};

//////////////
// MSGLOOP  !!
//////////////

#define SEND_ACTION WM_USER+1
#define QUERY_STATUS WM_USER+2

LRESULT CALLBACK message_window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProc(hwnd, message, wParam, lParam);
}

HWND create_message_window() {
    return ::CreateWindowEx(
        NULL,
        L"Message",
        L"CM15AMessageWindow",
        WS_CHILD,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        NULL,
        ::GetModuleHandle(NULL),
        NULL);
}


void run_message_loop(void* params) {
	CoInitialize(NULL);

    message_loop_is_running_ = false;

    cm15a_ = new cm15a_controller();
    if (cm15a_->get_error() == cm15a_controller::error_none) {
        message_hwnd_ = create_message_window();
        message_loop_is_running_ = true;

        ::SetEvent(reinterpret_cast<HANDLE>(params));

        MSG msg;
        bool done = false;
        while (( ! done) && (::GetMessage(&msg, NULL, 0, 0))) {
            switch (msg.message)
            {
            case SEND_ACTION:
            case QUERY_STATUS:
                {
                    critical_section cs(mutex_);

                    cm15a_message& nextMessage = queue_.front();

                    try {
                        if (msg.message == SEND_ACTION) {
                            cm15a_->send_action(nextMessage.type_, nextMessage.params_);
                        }
                        else if (msg.message == QUERY_STATUS) {
                            cm15a_->query_status(nextMessage.params_);
                        }
                    }
                    catch (...) {
                        // bleh. log?
                    }

                    queue_.pop();
                }
            break;

            case WM_QUIT:
                done = true;
                break;

            default:
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        message_loop_is_running_ = false;

        ::DestroyWindow(message_hwnd_);
        message_hwnd_ = NULL;
    }
    else {
        // failed to initialize the controller.
        ::SetEvent(reinterpret_cast<HANDLE>(params));
    }

    delete cm15a_;
    cm15a_ = NULL;

    ::CoUninitialize();

    ::SetEvent(quit_event_);
}

//////////////
// EXPORTS !!!
//////////////

CM15A_CONTROLLER_API
void set_message_received_callback(void* message_received_callback) {
    critical_section cs(mutex_);
	message_received_callback_ = (on_message_received_func) message_received_callback;
}

CM15A_CONTROLLER_API
bool send_device_message(const char* type, const char* params) {
    critical_section cs(mutex_);

    if (message_hwnd_) {
        queue_.push(cm15a_message(type, params));
        ::PostMessage(message_hwnd_, SEND_ACTION, 0, 0);

        return true;
    }

    return false;
}

CM15A_CONTROLLER_API
bool get_device_status(const char* device_address) {
    critical_section cs(mutex_);

    if (message_hwnd_) {
        queue_.push(cm15a_message("", device_address));
        ::PostMessage(message_hwnd_, QUERY_STATUS, 0, 0);

        return true;
    }

    return false;
}

CM15A_CONTROLLER_API
bool init() {
    critical_section cs(mutex_);

    if (reference_count_ == 0) {
        HANDLE reset_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);

        message_thread_ = CreateThread(
            NULL,
            NULL,
            (LPTHREAD_START_ROUTINE) &run_message_loop,
            reinterpret_cast<void*>(reset_event),
            0,
            NULL);

        // we do this to avoid a race condition where deinit() could be called
        // before the dummy window is actually created. this would cause window message
        // to never be received, and the message loop sould never start.
        ::WaitForSingleObject(reset_event, INFINITE);
        ::CloseHandle(reset_event);
    }

    ++reference_count_;

    return (message_loop_is_running_);
}

CM15A_CONTROLLER_API
void deinit() {
    critical_section cs(mutex_);

    if (reference_count_ == 0) {
        return;
    }

    --reference_count_;

    if ( ! reference_count_) {
		if (message_loop_is_running_) {
			::ResetEvent(quit_event_);
			::PostMessage(message_hwnd_, WM_QUIT, 0, 0);
			::WaitForSingleObject(quit_event_, INFINITE);
		}
    }
}

void raise_callback(const char** params, unsigned count) {
    critical_section cs(mutex_);

	if (message_received_callback_ != NULL) {
		message_received_callback_(params, count);
	}
}

//////////////
// ENTRY !!!!!
//////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return 1;
}

//////////////
// COM !!!!!!!
//////////////

cm15a_controller::cm15a_controller() 
: advise_cookie_(0)
, active_home_(NULL)
, active_home_events_(NULL)
, status_notifier_(NULL)
, error_(error_none) {
    HRESULT hr;
    _pAtlModule = &_Module;

    // create the "master" COM object
    hr = ::CoCreateInstance(
        __uuidof(ActiveHomeScriptLib::ActiveHome),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        __uuidof(ActiveHomeScriptLib::IActiveHome),
        (LPVOID *) &this->active_home_);

    if (FAILED(hr)) {
        error_ = error_failed_create_active_home;
        return;
    }

    // create an instance of our custom notification handler
    hr = CComObject<cm15a_status_notifier>::CreateInstance(&status_notifier_);

    if (FAILED(hr)) {
        error_ = error_failed_create_custom_notifier;
        return;
    }

    // grab an IActiveHomeEvents interface from our custom handler
    hr = status_notifier_->QueryInterface(
        __uuidof(ActiveHomeScriptLib::_IActiveHomeEvents),
        reinterpret_cast<void**>(&active_home_events_));

    if (FAILED(hr)) {
        error_ = error_failed_query_generic_notifier;
        return;
    }

    // link the active_home interface to the event handler
    hr = AtlAdvise(
        active_home_,
        active_home_events_,
        __uuidof(ActiveHomeScriptLib::_IActiveHomeEvents),
        &advise_cookie_);

    if (FAILED(hr)) {
        error_ = error_failed_atl_advise;
        return;
    }
}

cm15a_controller::~cm15a_controller() {
    if (active_home_) {
        AtlUnadvise(
            active_home_,
            __uuidof(ActiveHomeScriptLib::_IActiveHomeEvents),
            advise_cookie_);

        active_home_->Release();
    }

    if (active_home_events_) {
        active_home_events_->Release();
    }
}

bool cm15a_controller::send_action(const std::string& type, const std::string& params) {
    if (error_ == error_none) {
        try {
            active_home_->SendAction(
                _variant_t(type.c_str()),
                _variant_t(params.c_str()),
                _variant_t(),
                _variant_t());

            return true;
        }
        catch (...) {
        }
    }

    return false;
}

bool cm15a_controller::query_status(const std::string& device) {
    if (error_ == error_none) {
        try {
            std::string query = device + " on";
            _variant_t result = active_home_->SendAction(
                _variant_t("queryplc"),
                _variant_t(query.c_str()),
                _variant_t(),
                _variant_t());

            // emulate a message_received_callback_, report status.
            status_notifier_->raw_RecvAction(
                _variant_t("queryplc"),
                _variant_t(device.c_str()),
                _variant_t((result.intVal) == 1 ? "on" : "off"),
                _variant_t(""),
                _variant_t(""),
                _variant_t(""),
                _variant_t());

            return true;
        }
        catch (...) {
        }
    }

    return false;
}
