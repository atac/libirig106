
libirig106
==========

|StatusImage|_
|LicenseImage|

Forked from Bob Baggerman's `irig106lib`_

libirig106 is a cross-platform open-source library written in C for reading and
writing IRIG 106 Chapter 10 (now 11) files. Tested on all 3 major platforms.

A cross-platform `Python wrapper`_ is also available.


Using the library
-----------------

Basic usage consists of:

1. Opening the file
2. Reading a packet header
3. Processing packet data if relevant
4. Repeat from #2

As an example, to iterate over 1553 messages in a file:

.. code-block:: c

    I106Status status;
    I106C10Header header;
    MS1553F1_Message msg;

    int fd = open("test.c10", 0);
    while (!(status = I106NextHeader(fd, &header))){

        if (header.DataType == I106CH10_DTYPE_1553_FMT_1){  // 0x19

            // Read data into buffer
            int buffer_size = GetDataLength(&header);
            char *buffer = (char *)malloc(buffer_size);
            int read_count = read(fd, buffer, buffer_size);

            // Read CSDW and first message
            status = I106_Decode_First1553F1(&header, buffer, &msg);

            // Step through 1553 messages
            while (!status){
                // Process message...

                // Read next message
                status = I106_Decode_Next1553F1(&msg);
            }

            free(buffer);
        }

        else
            lseek(fd, GetDataLength(&header), SEEK_CUR);
    }

Data Formats
.............

Unlisted type numbers are reserved as of this writing.


====  ==================================================    =========
Type  Name                                                  Supported                      
====  ==================================================    =========
0x00  Computer-Generated F0 - User-Defined                  User-Defined
0x01  Computer-Generated F1 - Setup Record (TMATS)          Yes
0x02  Computer-Generated F2 - Recording Events              Yes
0x03  Computer-Generated F3 - Recording Index               Yes
0x04  Computer-Generated F4 - Streaming Config (TMATS)      No
0x09  PCM F1                                                Yes
0x11  Time Data F1                                          Yes 
0x12  Time Data F2                                          No
0x19  1553 F1                                               Yes
0x1A  1553 F2 - 16PP194                                     Yes
0x21  Analog F1                                             Yes
0x29  Discrete F1                                           Yes
0x30  Message F0                                            No
0x38  ARINC-429 F0                                          Yes
0x40  Video F0                                              Yes
0x41  Video F1                                              Yes
0x42  Video F2                                              No
0x43  Video F3                                              No
0x44  Video F4                                              No
0x48  Image F0                                              No
0x49  Image F1                                              No
0x4A  Image F2                                              No
0x50  UART F0                                               Yes
0x58  IEEE 1394 F0                                          No
0x59  IEEE 1394 F1                                          No
0x60  Parallel F0                                           No
0x68  Ethernet F0 - Ethernet Data                           Yes
0x69  Ethernet F1 - UDP Payload                             No
0x70  TSPI/CTS F0 - GPS NMEA-RTCM                           No
0x71  TSPI/CTS F1 - EAG ACMI                                No
0x72  TSPI/CTS F2 - ACTTS                                   No
0x78  Controller Area Network Bus                           Yes
0x79  Fibre Channel F0                                      No
0x7A  Fibre Channel F1                                      No
====  ==================================================    =========


Building and Testing (requires cmake)
--------------------

Unix / make
...........

::

    mkdir build && cd build
    cmake ..
    make

Windows / visual studio
.......................

From the VS native tools commandline run::

    mkdir build && cd build
    cmake ..
    cmake build . --config Release

Alternatively, run the build_and_test.py script from unix terminal or VS
commandline to build and then run the test suite.


.. _Python Wrapper: https://github.com/atac-bham/libirig106-python
.. _irig106lib: https://github.com/bbaggerman/irig106lib
.. |StatusImage| image:: https://img.shields.io/azure-devops/build/atac-bham/beac5449-743a-475e-9e57-8d3fcf729446/9
.. _StatusImage: https://dev.azure.com/atac-bham/libirig106/_build/latest?definitionId=9&branchName=master
.. |LicenseImage| image:: https://img.shields.io/github/license/atac/libirig106
