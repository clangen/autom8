#pragma once

#include <autom8/device/device_model.hpp>

namespace autom8 { namespace app {
    class DeviceEditOverlay {
        public:
            static void Create(autom8::device_model_ptr model);

            static void Edit(
                autom8::device_model_ptr model,
                const autom8::database_id deviceId);

            static void Delete(
                autom8::device_model_ptr model,
                const autom8::database_id deviceId);

        private:
            DeviceEditOverlay(
                autom8::device_model_ptr model,
                const autom8::database_id deviceId);

            autom8::database_id deviceId;
            autom8::device_model_ptr model;
    };
} }
