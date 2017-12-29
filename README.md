# Energy Consumption Measurement of C/C++ Programs using Clang Tooling

The green computing has an important role in today's software technology. Either speaking about small IoT devices or large cloud servers, there is a generic requirement of minimizing energy consumption. For this purpose, we usually first have to identify which parts of the system is responsible for the critical energy peaks. In this paper we suggest a new method to measure the energy consumption based LLVM/Clang tooling. The method has been tested on various open source systems and the output is visualized via the well-known Kcachegrind tool.


## Prerequisites
To run the project you need LLVM + Clang, the installation can be found here:
```
http://clang.llvm.org/get_started.html
```

## Configuration and building
Configuration is done simply using CMake. Make sure that the LLVM binaries are in your PATH: llvm-config is invoked during the configuration. 
Use GCC 4.8 or later to compile.

```
mkdir build
cd build
cmake ..
make or sudo make install
```

## Usage
Running the entire platform involves 3 steps.


### Instrumentation

Instrument a project in its own directory ("./"):
```
instru -l=5 -d="./" .cpp --
```

CRapl_Gen directory is created with index.txt !

You can also use the follow flags:
```
* -l = number of minimum statements;
* -o = output file;
* -d = directory;
* .cpp - extension.
```

### CRAPL

Install crapl libraries on the system:
```
cd Rapl\crapl
sudo make install
```

Edit makefile to get the libraries dependencies:
```
RAPL:= /home/username/Documents/Rapl
INCS := -I$(RAPL)
RAPLSRCS := ${RAPL}/crapl/measures.o ${RAPL}/crapl/rapl_interface.o ${RAPL}/crapl/rapl.o
SRCS := ${RAPLSRCS}
```

Execute your project:
```
make install
sudo modprobe msr
sudo <exec_file>
cd CRapl_Gen and check your output (<exec_file>.txt)
```


### Python Script
Now we need to convert the output results to see them in kcahegrind.

```
python toCallgrind.py -i ../tinyxml/CRapl_Gen/index.txt -f ../tinyxml/CRapl_Gen/xmltest.txt -d tinyxml -o kache.txt
```

You can also use the follow flags:
```
* -i = index of the functions
* -f = output file of the results;
* -d = directory;
* -o = output with callgrind format;
* -m = if you want to normalize the results you can give a file with the values of energy consumption just in main
```

### Kcachegrind visualization

First install it:
```
http://kcachegrind.sourceforge.net/html/Download.html
```
Then run:
```
Kcachegrind name_of_file.txt
```