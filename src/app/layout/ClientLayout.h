#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/adapter/DeviceListAdapter.h>
#include <app/util/Device.h>

namespace autom8 { namespace app {

    class ClientLayout:
        public cursespp::LayoutBase,
        public cursespp::ITopLevelLayout,
        public sigslot::has_slots<>
    {
        public:
            ClientLayout(autom8::client_ptr client);

            virtual void OnLayout() override;
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
            void OnDeviceRowActivated(cursespp::ListWindow* w, size_t index);

            std::shared_ptr<DeviceListAdapter> deviceListAdapter;
            autom8::client_ptr client;
            std::shared_ptr<cursespp::ListWindow> deviceList;
            cursespp::ShortcutsWindow* shortcuts;
    };

} }