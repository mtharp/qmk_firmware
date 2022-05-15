# IBM 327x 75-key

A 75-key beam spring keyboard supplied with the IBM 3276, 3277, 3278 and 3279
terminals, fitted with an Eaglesong Beamic converter.

* Keyboard Maintainer: [Michael Tharp](https://github.com/mtharp)
* Hardware Supported: *The PCBs, controllers supported*
* Hardware Availability: *Links to where you can find this hardware*

Make example for this keyboard (after setting up your build environment):

    make beamic/ibm327x_75key:default

Flashing example for this keyboard:

    make beamic/ibm327x_75key:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the top-left key and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB - some may have pads you must short instead
* **Keycode in layout**: Press the key mapped to `RESET` if it is available
