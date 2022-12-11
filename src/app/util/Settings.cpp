#include "Settings.h"
#include <f8n/str/utf.h>
#include <f8n/str/util.h>
#include <autom8/device/device_system.hpp>
#include <cursespp/Colors.h>

#ifndef WIN32
#include <unistd.h>
#endif

using namespace autom8;
using namespace f8n::sdk;
using namespace f8n::prefs;

namespace autom8 { namespace app { namespace settings {

    const std::string CLIENT_HOSTNAME = "client.hostname";
    const std::string CLIENT_PASSWORD = "client.password";
    const std::string CLIENT_PORT = "client.port";
    const std::string SERVER_ENABLED = "server.enabled";
    const std::string SERVER_PASSWORD = "server.password";
    const std::string SERVER_PORT = "server.port";
    const std::string SERVER_CONTROLLER = "server.controller";
    const std::string UI_BACKGROUND_TYPE = "ui.backgroundType";
    const std::string UI_COLOR_MODE = "ui.colorMode";
    const std::string UI_THEME = "ui.theme";

    static const std::string DEFAULT_UI_BACKGROUND_TYPE = "theme";
    static const std::string DEFAULT_UI_THEME = "default";

#ifdef WIN32
    static const std::string DEFAULT_UI_COLOR_MODE = "rgb";
#else
    static const std::string DEFAULT_UI_COLOR_MODE = "palette";
#endif

    void InitializeDefaults() {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            auto prefs = Prefs();
            prefs->SetDefault(CLIENT_HOSTNAME, "localhost");
            prefs->SetDefault(CLIENT_PASSWORD, "changeme");
            prefs->SetDefault(CLIENT_PORT, 7901);
            prefs->SetDefault(SERVER_ENABLED, true);
            prefs->SetDefault(SERVER_PASSWORD, "changeme");
            prefs->SetDefault(SERVER_PORT, 7901);
            prefs->SetDefault(SERVER_CONTROLLER, device_system::default_type());
            prefs->SetDefault(UI_BACKGROUND_TYPE, DEFAULT_UI_BACKGROUND_TYPE);
            prefs->SetDefault(UI_COLOR_MODE, DEFAULT_UI_COLOR_MODE);
            prefs->SetDefault(UI_THEME, DEFAULT_UI_THEME);
        }
    }

    std::shared_ptr<ISchema> Schema() {
        auto result = std::make_shared<TSchema<>>();
        result->Add(ClientSchema().get());
        result->Add(ServerSchema().get());
        result->Add(UiSchema().get());
        return result;
    }

    std::shared_ptr<ISchema> UiSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddEnum(
            UI_BACKGROUND_TYPE,
            { "inherit", "theme" },
            DEFAULT_UI_BACKGROUND_TYPE);
        result->AddEnum(
            UI_COLOR_MODE,
            { "basic", "rgb", "palette" },
            DEFAULT_UI_COLOR_MODE);
        result->AddEnum(
            UI_THEME,
            cursespp::Colors::ListThemes(),
            DEFAULT_UI_THEME);
        return result;
    }

    std::shared_ptr<ISchema> ClientSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddString(CLIENT_HOSTNAME);
        result->AddString(CLIENT_PASSWORD);
        result->AddInt(CLIENT_PORT);
        return result;
    }

    std::shared_ptr<ISchema> ServerSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddBool(SERVER_ENABLED);
        result->AddString(SERVER_PASSWORD);
        result->AddInt(SERVER_PORT);
        result->AddEnum(
            SERVER_CONTROLLER,
            device_system::types(),
            device_system::default_type());
        return result;
    }

    std::shared_ptr<Preferences> Prefs() {
        return Preferences::ForComponent("settings");
    }

    cursespp::Colors::BgType BackgroundType() {
        auto value = Prefs()->GetString(UI_BACKGROUND_TYPE);
        if (value == "inherit") {
            return cursespp::Colors::Inherit;
        }
        return cursespp::Colors::Theme;
    }

    cursespp::Colors::Mode ColorMode() {
        auto value = Prefs()->GetString(UI_COLOR_MODE);
        if (value == "rgb") {
            return cursespp::Colors::RGB;
        }
        else if (value == "palette") {
            return cursespp::Colors::Palette;
        }
        return cursespp::Colors::Basic;
    }

    static std::string GetPath(const std::string& sFile) {
        std::string sPath;
        int length;

#ifdef WIN32
        wchar_t widePath[2048];
        wchar_t* szFile = NULL;

        length = GetFullPathName(f8n::utf::u8to16(sFile).c_str(), 2048, widePath, &szFile);
        if (length != 0 && length < 2048) {
            sPath.assign(f8n::utf::u16to8(widePath).c_str());
            if (szFile != 0) {
                std::string sTheFile = f8n::utf::u16to8(szFile);
                sPath.assign(sPath.substr(0, length - sTheFile.length()));
            }
        }
        else {
            sPath.assign(sFile);
        }
#else
        char* szDir;
        sPath.assign(getcwd((char*)szDir, (size_t)length));
#endif

        return sPath;
    }

    std::string GetApplicationDirectory() {
        std::string result;

#ifdef WIN32
        wchar_t widePath[2048];
        int length = GetModuleFileName(NULL, widePath, 2048);
        if (length != 0 && length < 2048) {
            result.assign(GetPath(f8n::utf::u16to8(widePath).c_str()));
        }
#elif __APPLE__
        char pathbuf[PATH_MAX + 1];
        uint32_t bufsize = sizeof(pathbuf);
        _NSGetExecutablePath(pathbuf, &bufsize);
        result.assign(pathbuf);
        size_t last = result.find_last_of("/");
        result = result.substr(0, last); /* remove filename component */
#else
        char pathbuf[PATH_MAX + 1] = { 0 };

#ifdef __FreeBSD__
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        size_t bufsize = sizeof(pathbuf);
        sysctl(mib, 4, pathbuf, &bufsize, nullptr, 0);
#elif defined  __OpenBSD__
        int mib[4];
        char** argv;
        size_t len = ARG_MAX;

        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC_ARGS;
        mib[2] = getpid();
        mib[3] = KERN_PROC_ARGV;

        argv = new char* [len];
        if (sysctl(mib, 4, argv, &len, nullptr, 0) < 0) abort();

        fs::path command = fs::absolute(fs::path(fs::u8path(argv[0])));
        realpath(command.u8string().c_str(), pathbuf);
        delete[] argv;
#else
        std::string pathToProc = f8n::str::format("/proc/%d/exe", (int)getpid());
        readlink(pathToProc.c_str(), pathbuf, PATH_MAX);
#endif

        result.assign(pathbuf);
        size_t last = result.find_last_of("/");
        result = result.substr(0, last); /* remove filename component */
#endif

        return result;
    }

}}}
