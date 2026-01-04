build:
	pio run
flash:
	pio run --target upload
start-gdb-server:
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
debug:
	arm-none-eabi-gdb -q -x init.gdb .pio/build/bluepill_f103c8/firmware.elf

.PHONY: build flash start-gdb-server debug

