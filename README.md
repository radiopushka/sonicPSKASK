This is a program designed to demonstrate the possibility of sending information between two devices via ultrasound. This could be two laptops or anything that supports alsa and Linux.

after you run make you will get two binary files, modrx and modgen. you can pass the text you want to send to modgen. The text should be short, 4 characters is a good amount. It uses the default device.
./modgen hello

you can run ./modrx and it will listen for any packets using the microphone under alsa's default device.

This code is a work in progress and updates may occur but I do not have much time so the might be sparse. There are still things I would like to sharpen like the peak detector and synchronizer in the modulator.c file in the library directory.

This code is licensed under GPL v3
