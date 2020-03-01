# Lunatico Astronomy Limpet/Armadillo/Platypus rotator/focuser driver

https://www.lunatico.es

## Supported devices

Seletek, Armadillo, Platypus and Limpet controllers.

No hot plug support. All devices are present at startup:

* **Limpet** - One rotator is present.
* **Seletek** and **Armadillo** - One rotator is present on "Main" port and "Exp" can be configured as rotator or focuser.
* **Platypus** - One rotator is present on "Main" port, while "Exp" and "Third" can be configured as rotator or focuser.

## Supported platforms

This driver is supported on Linux (Intel 32/64 bit and ARM v6+) and MacOS.

## License

INDIGO Astronomy open-source license.

## Use

indigo_server indigo_rotator_lunatico

## Status: Under Development

Driver is developed and tested with:
* Armadillo controller

## NOTE:
This driver can not be loaded if *indigo_focuser_lunatico* is loaded as both drivers have the same functionality.
The only difference is that "Main" port of *indigo_focuser_lunatico* driver is configured as focuser.
