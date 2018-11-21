#include "DeviceEditOverlay.h"

using namespace autom8;
using namespace autom8::app;


void DeviceEditOverlay::Create(autom8::device_model_ptr model) {
}

void DeviceEditOverlay::Edit(device_model_ptr model, const database_id deviceId) {

}

void DeviceEditOverlay::Delete(device_model_ptr model, const database_id deviceId) {

}

DeviceEditOverlay::DeviceEditOverlay(device_model_ptr model, database_id deviceId)
: model(model)
, deviceId(deviceId) {

}