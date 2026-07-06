# About a project
This is sources for arduino scheme with 4-digit 7-segment display and a button for mode changing.
# Sources
* **arduino_sketch.ino** contains arduino source code firmware. Use it to flash your arduino. Change pin macros at the beggining of the file.
* **main.c** contains host program that make a connection with arduino.
# Install
To install program from sources use ```sudo make install```. You also can uninstall program with ```sudo make uninstall```. To change installation root directory you can define **DESTDIR** variable.
Alternatively if you're using NixOS you can install program by importing module.nix as a module in your **configuration.nix** file and then write ```services.artemp.enable = true;```
