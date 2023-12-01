# picoDDS
Raspberry Pico example with Pio, DMA and Vcore usage to build a DDS with console integration

# Setup
- Download the pico-sdk repository
- Define PICO_SDK_PATH in your ~/.bashrc• 

# Build
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
Go to the specific folder and

`make -j4`

# Visual Studio Code integration
Install the extensions as explained in the  [Pico Getting started manual](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)

Or just download [VScode](https://code.visualstudio.com/Download) and add the following extension

```
code --install-extension marus25.cortex-debug
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.cpptools
```

# Debug
Install open ocd as explained in the [Pico Getting started manual section Installing OpenOCD](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)

#Using Pico probe
On the same document check **Appendix A: Using Picoprobe**

Open deploy and run
```
openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program <specific elf file> verify reset exit"
```
