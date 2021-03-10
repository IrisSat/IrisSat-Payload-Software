# IrisSat-PayloadSoftware
Software for IrisSat Payload Controller.

Branches:
canEx1_nucleo -> Should have working can code.
CAN -> merged canEx1_nucleo, but not sure if merged code will work...
master-> workign code with adc reading.

## Building and Running

The project is setup for the STM32CubeIDE.
Build the porject by clicking the "hammer" icon in the top toolbar, or by right clicking the project in the Project Explorer and selecting "Build Project".

The project can be run by connecting the debugger, and then clicking the debug symbol in the top toolbar.

## Configuration

Here is info about how the project is configured.
The project was initially generated with the CubeMX tool.
These settings should now be changed in the software, not by re-generating the code using CubeMX, becauase it can overwrite any code we write.


### FreeRTOS
 Configuration options for FreeRTOS are in the file FreeRTOSConfig.h.
- The project is setup for FreeRTOS version 10.2.1.
- Tick rate is 1kHz.
- The heap size is 15 kbytes and uses heap_4.
- All the usual features are enabled.

### CAN Bus
- The CAN peripheral is setup in slave mode, with the default options.
- The RX interrupts are enabled.

### NAND Flash (FMC)
- The memory peripheral  is configured with the NAND characteristics (page size, block size, etc).
- The timing options have not been modified. These will probably need to be changed based on info in the flash datasheet.

### DCIM Camera Interface
- The DCIM peripheral is set for an 8-bit device with external synchronization.
- The sync signals are set active high and the clock is active on the rising edge.
- JPEG mode is disabled.

### I2C Camera Interface
- The I2C bus for each camera is set up with the standard options and a 100kHz clock speed.

### GPIO Pins
- The GPIO pins for the multiplexer signals have been assigned and labeled.
- The ADC inputs are assigned with the default settings.

### Clock Configuration
- The project is setup to use the 25MHz external clock.
- The main clock speed is 200MHz, and the other clock signals are derived using the standard prescalers.


