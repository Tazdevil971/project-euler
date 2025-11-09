# Design errata
## Rev0
|Mistake|Notes|Possible fix|
|-------|-----|------------|
|`IMU_INT`/`IMU_BOOTN` flipped in schematic||Flip it software side|
|`IMU_RST` doesn't behave nominally|For some reason, reset pin is stuck at 0.7v even if the pull-down resistor is installed, this is probably due to an internal pull-up inside the BNO086|Remove the pull-down and use deep sleep GPIO hold on the MCU|
|`USB_AWAKE` is left floating|This can cause the USB to never fully go into sleep when disconnected and with the MCU in deep sleep|Use deep sleep GPIO hold on the MCU|