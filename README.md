# Exmaple IoT Firmware <img align="right" src="./readme_images/picmg_logo.png" width=8% height=8%><img align="right" src="./readme_images/SmartSensorLogo.png" width=20% height=20%>

## Overview

This project implements example firmware that meets the requirements found in the PICMG(R) IoT.1 (IoT Firmware) specification. More information about PICMG IoT.1 can be found on the PICMG website (www.picmg.org).  The firmware is one of several references implementations provided by PICMG to demonstrate implementation of the IoT specifications.

The use case enabled by the PICMG firmware specification involves a user who wishes to create a smart sensor but does not nessarily have the proficiency to create firmware code. The user to creates a firmware configuration using a configurator tool.  The configurator tool, in turn, creates a configuration file that drives the rest of the firmware build process.  This use-case is shown in the following image.

<img align="center" src="./readme_images/UseCase.png" width=60% height=60%>

**The PICMG Firmware specification does not dictate the target hardware, however, this code assumes a PICMG MicroSAM module based on the Atmega 328PB microcontroller.**

The primary features of this firmware are:
- Numeric and sensors/effecters with linearization
- State sensors / effecters with possible OEM-defined state sets
- Synchronization using global interlock and trigger signals
- Profiled motion control with limit sensors and either S-curve or Trapezoidal motion profiles
- Simple sensor / effecter readings and control
- Fru data support
- Device discovery through use of MCTP Discovery Notify message
- Full PDLM communications stack over MCTP/Serial

Other example code from PICMG can be found here:
- https://github.com/PICMG/iot_configurator.git - example configurator that allows the user to create constraints-based device configurations for the firmware
- https://github.com/PICMG/iot_builder - example code that converts configurator output (config.json) to C code that completes the firmware configuration for a Atmega 328PB-based firmware build.  This code can be adapted to other microcontrollers with minor modification.

## Build Tools

The PICMG IoT firmware was developed using Visual Studio Code on Linux, and built using the GNU toolchain for avr microcontrollers.  The target hardware was assumed to be a PICMG MicroSam module based on the Atmega328PB microcontroller.  Alternately, an Arduino nano may be used for testing with reduced functionality.  Programming the firmware image onto the device will require the Avrdude utility.

### Prerequisites:

- Linux OS running Ubuntu 20.04.1 LTS (Focal Fossa).  Other versions of Linux may work but may also require other configuration steps.
- Atmega 328PB-based MicroSAM module (or Arduino Nano for testing with reduced functionality)
- PICMG IoT Builder (if new configurations are desired)
- PICMG IoT Configurator (if new configurations are desired)

### Install Build Tools

Enter the following commands on your linux build system to load the required packages for building the firmware

> sudo apt install make

> sudo apt install g++
 
> sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc avrdude

### Visual Studio Code setup (for Development)

Install the Linux GitHub package for Visual Studio Code
1.	Download and install Visual Studio Code from the Ubuntu Software Center.
2.	Open Visual Studio Code and hit Ctrl+Shift+X to open the extensions window.
3.	Search for and install the C/C++ and C++ Intellisense extensions.
4.	Once the extensions are finished installing, open the Explorer menu at the top left of the screen and click Clone Repository.
5.	In the text box in the top center of the screen, enter the URL for this repository.
6.	When prompted for authentication, follow the on screen directions to authenticate with GitHub using your username and password.
7.	Once the repository clone is done, click on the Source Control tab on the left side of the screen and initialize the repository.

## Build Process

The firmware currently supports two operating modes: profiled motion with a stepper controller, and a simple sensor with a single thermocouple device.  The reference configuration files for each of these can be found in the ./reference_configuration folder of this project. To build the configuration for the stepper configuration, change the working directory to ./avr/test/userver and invoke the following command:

> make run_stepper

This will build and attempt to load the resulting hex file onto your target hardware located at /dev/ttyUSB0.  If your target is located on another device, the location can easily changed by modifying the Makefile.
To build the configuration for the simple sensor model, set your current working directory to ./avr/test/userver and invoke:

>make run_simple

This will build and attempt to load the resulting hex file onto your target hardware located at /dev/ttyUSB1
