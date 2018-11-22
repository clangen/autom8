#pragma once

#include <autom8/net/client.hpp>
#include <autom8/device/device_system.hpp>
#include <cursespp/ScrollAdapterBase.h>
#include <functional>
#include <mutex>

namespace autom8 { namespace app {
    class DeviceModelAdapter:
            public sigslot::has_slots<>,
            public cursespp::ScrollAdapterBase
    {
        public:
            DeviceModelAdapter(autom8::device_system_ptr);
            virtual ~DeviceModelAdapter();

            virtual size_t GetEntryCount() override;

            virtual cursespp::IScrollAdapter::EntryPtr GetEntry(
                cursespp::ScrollableWindow* window, size_t index) override;

            const autom8::device_ptr At(const size_t index);

            void Requery();

        private:
            autom8::device_system_ptr system;
            autom8::device_list devices;
    };
} }