#//Evan Nikitin Wed Oct  2 09:22:05 PM -00 2024
mod_test:
	cc example.c SyncedPSKASK_modulationLibrary/modulator.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_arrays.c SyncedPSKASK_modulationLibrary/Freq_lib/frequency_itr.c SyncedPSKASK_modulationLibrary/Freq_lib/convolution/lowpass.c -g -march=native -O2 -lm -o modtest
