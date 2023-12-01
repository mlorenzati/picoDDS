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

# Schematic
Just build a simple R2R ladder for the 2 channels
![image](https://github.com/mlorenzati/picoDDS/assets/5400635/4a0bd436-a1dc-4b54-b9d4-6b9e52f45919)

# Usage
Deploy uf2 to the pico and use a serial terminal to interface with the console
![image](https://github.com/mlorenzati/picoDDS/assets/5400635/e2a7d6f4-1720-4213-b8df-2bf5e9cbf702)

To know commands just hit enter
Commands: add(a) <value>, channel(c) <value>, frequency(f) <value>, multiply(m) <value>, offset(o) <value>, reset(r) , waveform(w) <value>

