# Arduino-MatrixControl
MatrixControl is an [Arduino](http://arduino.cc) library for MAX7219 and MAX7221 Led Matrix display drivers.   

# Background
This library was developed based on [this](https://github.com/wayoda/LedControl).   
The original library also supports 8-segment LEDs, but this library does not support 8-segment LEDs.   
Only supports cascaded 8x8 Led Matrix.   

![IMG_1784](https://user-images.githubusercontent.com/6020549/125252509-6dce3580-e333-11eb-813c-71ad5090ccfc.JPG)

# Change from original
While the original library is highly versatile, the processing of cascaded LEDs is inefficient.   
This library speeds up the processing of cascaded LEDs.   

The original library used software SPI, but this library can use both software SPI and hardware SPI.   

- Software SPI(Default)   
```#define BITBANG 1```

- Hardware SPI
```#define BITBANG 0```


## Processing time using LedControl-Scroll-FullSpeed
diffMillis[LedControl]=1973[ms]
diffMillis[LedControl]=1973[ms]
diffMillis[LedControl]=25565[ms]
diffMillis[LedControl]=25574[ms]

## Processing time using MatrixControl-Scroll-FullSpeed
diffMillis[MatrixControl]=293[ms]
diffMillis[MatrixControl]=292[ms]
diffMillis[MatrixControl]=1093[ms]
diffMillis[MatrixControl]=1094[ms]

# Documentation
Documentation for the library is on the [Github Project Pages](http://wayoda.github.io/MatrixControl/)

# Install
The library can be installed using the [standard Arduino library install procedure](http://arduino.cc/en/Guide/Libraries)  

# Wiring
See source code.

# Normal display
![IMG_1776](https://user-images.githubusercontent.com/6020549/125252563-79b9f780-e333-11eb-907a-43768da36622.JPG)
![IMG_1777](https://user-images.githubusercontent.com/6020549/125252568-7b83bb00-e333-11eb-965e-a5890266aa68.JPG)
![IMG_1778](https://user-images.githubusercontent.com/6020549/125252573-7cb4e800-e333-11eb-8cf2-5c3bbf9e5fef.JPG)

# Invert display
![IMG_1779](https://user-images.githubusercontent.com/6020549/125252680-99512000-e333-11eb-8ad1-076e0df81ec1.JPG)
![IMG_1780](https://user-images.githubusercontent.com/6020549/125252695-9d7d3d80-e333-11eb-9e25-921b95c4f78b.JPG)

