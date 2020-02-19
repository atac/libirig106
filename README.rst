
libirig106
==========

|StatusImage|_

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

    if ((status = I106C10Open(&handle, filename, READ))){
        printf("Error opening file!");
        return;
    }

    while (!(status = I106C10ReadNextHeader(handle, &header)){
        int buffer_size = GetDataLength(&header);
        char *buffer = malloc(buffer_size);

        if ((status = I106C10ReadData(handle, buffer_size, buffer))){
            printf("Error reading data.");
            break;
        }

        if (header.DataType == I106CH10_DTYPE_1553_FMT_1){  // 0x19
            while ((status = I106_Decode_First1553F1(&header, buffer, &msg)){
                // Process message...
            }
        }
    }

    I106C10Close(handle);

Directory Structure
...................

The top-level capability is in the "irig106ch10" file while datatype-specific
parsing can be found in the "i106_decode_X" files where "X" is the datatype in
question.

Supported data types:

* 1553 Format 1
* Analog Format 1
* Arinc429 Format 0
* CAN
* Discrete Format 1
* Ethernet Format 0
* Index packets
* PCM Format 1
* Time packets
* TMATS packets
* UART Format 0
* Video Format 0

To Do
-----

* Explore difference between rel_time and RTC (verbage is inconsistent esp. in
  i106_time)
* More robust tests
* Remove/replace Message->Datalength (refers to packet body size as opposed to
  message length)
* Implement support for index records.
* Implement seek() based on time.
* Parse more TMATS fields
* Provide better, more automatic ways to keep time in sync
* Review "header version" to see what needs to be accounted for in code
* Implement partial & missing datatypes


.. _Python Wrapper: https://github.com/atac-bham/libirig106-python
.. _irig106lib: https://github.com/bbaggerman/irig106lib
.. |StatusImage| image:: https://dev.azure.com/atac-bham/libirig106/_apis/build/status/atac-bham.libirig106?branchName=master
.. _StatusImage: https://dev.azure.com/atac-bham/libirig106/_build/latest?definitionId=2&branchName=master
