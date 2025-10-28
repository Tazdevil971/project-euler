|Pin NO|Function|Connected to|Note|
|------|--------|------------|----|
|GPIO0|Input IRQ, active low, open drain|USB sleep|Available in ULP subsystem|
|GPIO1|Input IRQ, active low, open drain|USB pwren|Available in ULP subsystem|
|GPIO2|Input IRQ, active low, open drain|USB BCD|Available in ULP subsystem|
|GPIO3|Input IRQ, active low, open drain|BQ25186 interrupt|Available in ULP subsystem|
|GPIO4|ADC1 CH4|VBAT sense|Resistor divider using 10k and 33.2k|
|GPIO5|Input IRQ, active low|User button 1|Available in ULP subsystem|
|GPIO6|I2C SDA|I2C low power bus (BQ25186)|Available in ULP subsystem|
|GPIO7|I2C SCL|I2C low power bus (BQ25186)|Available in ULP subsystem|
|GPIO8|Output, active low|User LED 1||
|GPIO9|Boot mode selector|||
|GPIO10|Output, active low|User LED 2||
|GPIO11|Input IRQ, active low|User button 2||
|GPIO15|Input IRQ, active low, open drain|USB I2C RXF||
|GPIO16|UART0 TX|||
|GPIO17|UART0 RX|||
|GPIO18|Output, active low|USB keep awake||
|GPIO19|I2C SDA|I2C normal bus (USB controller, BNO086)||
|GPIO20|I2C SCL|I2C normal bus (USB controller, BNO086)||
|GPIO21|Output, active high|BNO086 BOOTN||
|GPIO22|Output, active low|BNO086 reset||
|GPIO23|Input IRQ, active low, (open drain?)|BNO086 interrupt||