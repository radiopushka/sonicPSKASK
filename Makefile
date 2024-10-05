#//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024
mod_test:
	cc transmitter.c alsa/alsa.c SyncedPSKASK_modulationLibrary/modulator.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_arrays.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_itr.c SyncedPSKASK_modulationLibrary/Freq_lib/convolution/lowpass.c -g -march=native -O2 -lasound -lm -o modgen
	cc receiver.c alsa/alsa.c SyncedPSKASK_modulationLibrary/modulator.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_arrays.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_itr.c SyncedPSKASK_modulationLibrary/Freq_lib/convolution/lowpass.c -g -march=native -O2 -lasound -lm -o modrx
