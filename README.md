# STM32F030 & TLC5540 OSCILLOSCOPE

There are many arduino-based simple oscilloscopes with max 200 kilosamples per
second. This is sufficient mainly to audio signals (top comprehensive signal
frequency is a half of sampling speed, so practical maximum is 70 kHz). Meanwhile many
tasks require faster signal sampling. E.g. creating generator for Middle or
Short-Wave transmitter (even not speaking of higher ranges) - or just analyzing
signals in digital designs, where 1MHz is nothing special.

So this project is an attempt to build a faster signal sampler, to be used
as a basic oscilloscope.

### TLC5540

This ADC has just 8bit resolution, but for oscilloscope that is nice. Meanwhile
it allows up to 40 megasamples per second. This is good, though no basic
controllers can retrieve data with such speed.

### STM32F030

This is a simple, very cheap 32-bit controller, running at 48MHz. It's most
handy package has pins at 0.8 mm pitch, so it is yet normal for home-made design.
However, of course, controller couldn't fetch data from ADC at every clock, so
you may try more advanced chip running at higher speed (some of STM32F4... for example).

### Current solution

Currently device is capable of fetching data at 8 megasamples. This allows
analyzing signals of up to 3 MHz (i.e. 40 times better compared to arduino-based
solutions).
