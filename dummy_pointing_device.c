#include "pointing_device.h"

void pointing_device_driver_init(void){
    return;
}
report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report){
    return mouse_report;
}
uint16_t pointing_device_driver_get_cpi(void) {
    return 0;
}
void pointing_device_driver_set_cpi(uint16_t cpi){
    return;
}