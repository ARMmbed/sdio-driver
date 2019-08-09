# SDIO-driver

SDIO block device driver. 

This is not officially supported driver.

## License & Contributions 

The software is provided under [Apache-2.0 license](LICENSE). Contributions to this project are accepted under the same license. Please see [contributing.md](CONTRIBUTING.md) for more info.

This is an implementation for a blockdevice to use SDHC cards via SDIO interface.

## Supported Hardware
- STM32F4
- STM32F7
- LPC55S69

## Tested Hardware
- STM Discovery board 32F469IDISCOVERY, Targetname: DISCO_F469NI
  https://os.mbed.com/platforms/ST-Discovery-F469NI/
- STM Discovery board 32F746IDISCOVERY, Targetname: DISCO_F746NG
  https://os.mbed.com/platforms/ST-Discovery-F746NG/
- NXP LPCXpresso55S69 board, Targetname: LPC55S69_NS
  https://os.mbed.com/platforms/LPCXpresso55S69/  

## Driver Description 

This SDIOBlockdevice inherits from Blockdevice. The implementation uses an intermediate layer for the target specific code.

## TARGET_STM Support

This version uses the STM HAL for the SD card communication, an approach that is not fully mBed compliant. So this driver is not part of the mbed-os, a cleaner solution maybe supplied in the future. Anyway, this driver passes the same tests as for a SDBlockdevice with SPI interface.

## Usage

The driver is used like other blockdevices, a good starting point is https://github.com/ARMmbed/mbed-os-example-filesystem


