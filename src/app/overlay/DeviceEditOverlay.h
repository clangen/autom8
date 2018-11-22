#pragma once

#include <cursespp/OverlayBase.h>
#include <cursespp/ShortcutsWindow.h>
#include <autom8/device/device_system.hpp>
#include <functional>

namespace autom8 { namespace app {
    class DeviceEditOverlay: public cursespp::OverlayBase, public sigslot::has_slots<>
#if (__clang_major__ == 7 && __clang_minor__ == 3)
        , public std::enable_shared_from_this<DeviceEditOverlay>
#endif
{
        public:
            static void Create(autom8::device_system_ptr system);

            static void Edit(
                autom8::device_system_ptr system,
                autom8::device_ptr device);

            static void Delete(
                autom8::device_system_ptr system,
                autom8::device_ptr device,
                std::function<void()> callback);

            DeviceEditOverlay(
                autom8::device_system_ptr system,
                autom8::device_ptr device = autom8::device_ptr());

            virtual void Layout() override;
            virtual bool KeyPress(const std::string& key) override;

        private:
            using Label = std::shared_ptr<cursespp::TextLabel>;
            using Input = std::shared_ptr<cursespp::TextInput>;
            using Shortcuts = std::shared_ptr<cursespp::ShortcutsWindow>;

            void LayoutLabelAndInput(Label label, Input input, int y, int inputWidth = -1);
            void UpdateOrAddDevice();

            autom8::device_ptr device;
            autom8::device_system_ptr system;

            autom8::device_type deviceType;
            Label titleLabel;
            Label addressLabel, labelLabel, groupsLabel, typeLabel;
            Input addressInput, labelInput, groupsInput;
            Shortcuts shortcuts;
    };
} }
