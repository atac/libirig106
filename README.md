# IRIG106LIB

[![Build Status](https://dev.azure.com/atac-bham/libirig106/_apis/build/status/atac-bham.libirig106?branchName=master)](https://dev.azure.com/atac-bham/libirig106/_build/latest?definitionId=2&branchName=master)

Originally created by Bob Baggerman <bob@irig106.org>

irig106lib is an open source library for reading and writing IRIG 106 
Chapter 10 files. The library is compatible with Windows/Visual C or
Unix/GCC environments. A cross-platform Python wrapper is also available.


## Using the library

Reading files involves opening the file, reading a packet header, 
optionally reading the packet body (which may contain one or more data messages), 
decoding the data, and then looping to read the next header. The 
routines for handling data packets are in "irig106ch10". Routines for decoding 
each data type are organized into files named for the corresponding data
type. For example, 1553 decoding is contained in "i106_decode_1553f1". Below is
a simplified example of packet processing:

``` .c
    I106Status status = I106C10Open(&handle, filename, READ);
    I106C10Header header;

    while (status == I106_OK){
        status = I106C10ReadNextHeader(handle, &header);

        if (status == I106_EOF)
            break;

        int buffer_size = GetDataLength(&header);
        char *buffer = malloc(buffer_size);
        status = I106C10ReadData(handle, buffer_size, buffer);

        MS1553F1_Message msg;

        if (header.DataType == I106CH10_DTYPE_1553_FMT_1){  // 0x19
            status = I106_Decode_First1553F1(&header, buffer, &msg);
            while (status == I106_OK){
                // Process message...
                status = I106_Decode_Next1553F1(&msg);
            }
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
* int.h - Standard integer definions for environments that don't supply this.


## To Do

### First Pass (current)

* Explore difference between rel_time and RTC (verbage is inconsistent esp. in
  i106_time)
* Automated tests
* Remove/replace Message->Datalength (refers to packet body size as opposed to
  message length)

### Other

* Update and spinoff utils?
* Implement support for index records.
* Implement seek() based on time.
* Implement video decoder
* Parse more TMATS fields
* Provide better, more automatic ways to keep time in sync
* Review "header version" to see what needs to be accounted for in code
* Implement partial & missing datatypes
    * 1394
