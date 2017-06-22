# IRIG106LIB

Copyright (c) 2017 Irig106.org

Originally created by Bob Baggerman <bob@irig106.org>

irig106lib is an open source library for reading and writing IRIG 106 
Chapter 10 format files. The library supports the following compilers (building
a static library):

* Microsoft Visual C (Win32 static library and DLL)
* GNU GCC compiler (Linux and macOS)
* DJGPP

A Python wrapper for the compiled DLL is also included.  The Python wrapper is
incomplete but demonstrates how to make calls into the DLL from Python.


## Using the library

Reading files involves opening the file, reading a data packet header, 
optionally read the data packet (which may contain multiple data packets), 
decode the data packet, and then loop back and read the next header.  The 
routines for handling data packets are in "irig106ch10".  Routines for decoding 
each data packet type are contained in their own source code modules.  For 
example, 1553 decoding is contained in "i106_decode_1553f1".  Below is a 
simplified example of packet processing:

``` .c
I106C10Open(&handle, file, I106_READ);

while (1){
    I106Status status = I106C10ReadNextHeader(handle, &header);

    if (status == I106_EOF) return;

    status = I106C10ReadData(handle, &buffer_size, buffer);

    switch (header.DataType){

        case I106CH10_DTYPE_1553_FMT_1 :    // 0x19

            status = I106_Decode_First1553F1(&header, buffer, &msg);
            while (status == I106_OK){
                // Do some processing...
                status = I106_Decode_Next1553F1(&msg);
            }
            break;
        default:
            break;
    }
}

I106C10Close(handle);
```


## Modules

### Core Modules

Core software modules support opening data files for reading and 
writing, and working with headers and data at a packet level.  These 
software modules must be included in any program that uses the IRIG 
106 software library.  Core software modules include:

* irig106ch10 - The main source module containing routines for opening, reading, 
writing, and closing data files are contained in "irig106ch10.c".  Other 
software modules are provided to handle the various IRIG 106 Ch 10 packet 
formats.
* i106_time - Routines to convert between clock time and IRIG 106 time counts
* i106_index - A higher level interface to the indexing system
* i106_data_stream - Support for receiving Chapter 10 standard UDP data packets
  (currently disabled)


### Decode Modules

Decode software modules are used to decode data of a specific type.  
Decode modules have names of the form "i106_decode_*" where the "*" 
describes the type of data handled in that module.  Only those decoder 
modules that are used need to be included in your software project.  
Modules for unused data types can be omitted.  Decoder modules 
include:

* i106_decode_tmats - Decode a TMATS data packet into a tree structure for
easy interpretation.
* i106_decode_time - Decode IRIG time packets and provide routines for
converting relative time count values to IRIG referenced time.
* i106_decode_1553f1 - Decode all 1553 format packets.
* i106_decode_arinc429 - Decode ARINC 429 format packets
* i106_decode_discrete - Decode descrete format packets
* i106_decode_ethernet - Decode Ethernet format packets
* i106_decode_index - Decode index packets
* i106_decode_uart - Decode serial UART format packets
* i106_decode_video - Decode video format packets


### Other Headers

These header files are necessary for every application that uses the IRIG 106 library.

* config.h - A bunch of #defines to support various compiler environments.
* stdint.h - Standard integer definions for environments that don't supply this.


## To Do

### First Pass (current)

* Update naming conventions (need to codify somewhere)
* Disable networking, C++, and lookahead code (for now)
* Move util functions from top-level to new util "module"
    * Create valid_handle utility function to reduce duplication
* Explore difference between rel_time and RTC (verbage is inconsistent esp. in
  i106_time)
* Automated tests

### Other

* Update and spinoff python wrapper
* Update and spinoff utils?
* Implement support for index records.
* Implement seek() based on time.
* Implement video decoder
* Parse more TMATS fields
* Provide better, more automatic ways to keep time in sync
* Review "header version" to see what needs to be accounted for in code
