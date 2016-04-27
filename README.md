EasyTransferPP Arduino Library
==============================

EasyTransfer allows easy transfer of a C data structure using any stream class.

It is based on EasyTransfer protocol 
<http://www.billporter.info/easytransfer-arduino-library/>
by Mathieu Alorent and Bill Porter <www.billporter.info>.

Usage
=====

You can define a struct two send complex data structures:
```cpp
struct mystruct_t {
 int a;
 long b;
};
```

Define a ETPP object, initialized with your stream object. For example,

```cpp
EasyTransferPP<TwoWire> easyWire(Wire);
```

The first argument, `TwoWire`, is the type of TWI `Wire`.

Another example is
```cpp
EasyTransferPP<Stream> easySerial(Serial);
```

If you always expect to receive the same data, you can define it in your object.
For example, here the `easyWire` expects to receive a `mystruct_t`:
```cpp
EasyTransferPP<TwoWire, mystruct_t> easyWire(Wire);
```

Sending data
------------

To send data, you have the following options available:
```cpp
void send(const T  &data_struct);
void send(uint8_t const * const data_pointer, uint8_t size);

void sendTo(uint8_t address, const T  &data_struct);
void sendTo(uint8_t address, uint8_t const * const address, uint8_t size) ;
```

Here:
- `send` is used for `Serial`, while `sendTo` is used for `Wire`.
- `address` is the receivers address in TWI.
- `data_pointer` and `size` can be used to send any buffer.
- `T` is automatically converted to your data type using templates.

For example, to send
```cpp
mystruct_t sample_data;
```
you only have to call
```cpp
easySerial.send(sample_data);
```

Receive buffers
---------------

As observed above, the send function can use any data structure.
But to receive data correctly, it's type has to be set.

EasyTransferPP allows setting an internal buffer, using the object Constructor (e.g., 
`EasyTransferPP<TwoWire, mystruct_t> easyWire(Wire)`).

Alternatively, you can set your own receive buffers:
```cpp
void setReceiveBuffer(T &data_struct);
void setReceiveBuffer(uint8_t * data_pointer, uint8_t size);
```

To get a pointer to this buffer, you can use `receiveBuffer()`:
```cpp
const mystruct_t & x = easyWire.receiveBuffer();
```
You can drop the `const` and `&` if you don't care about const correctness.

Or if your compiler supports `auto` variable type definition:
```cpp
auto x = easyWire.receiveBuffer();
```

Receive operation
-----------------

To receive data, call `receive()`. It automatically reads available bytes from
the Serial or TWI object, and returns TRUE if the receive operation is complete.

For example,
```cpp
#include <Wire.h>
#include "ETPP.h"

struct mystruct_t {
 int a;
 long b;
};

EasyTransferPP<TwoWire, mystruct_t> easyWire(Wire);

void setup() {

  Serial.begin(9600); // this is used for debugging

  Wire.begin(64); // begin as slave 64
  Wire.onReceive(onreceive);
}

// Callback function for when data is received
void onreceive(int numbytes) {
  bool frameCompleted = easyWire.receive();
  
  if (frameCompleted) {
    // get the results from internal buffer
    auto request = easyWire.receiveBuffer();
    
    Serial.println("data received");
    Serial.print("a: ");
    Serial.println(request.a);
    Serial.print("b: ");
    Serial.println(request.b);
  }  else {
    Serial.println("Error while receiving the ET frame.");
  }
}


void loop() {

}

```

To request data
---------------

In TWI, you can request data from a slave:

```cpp
bool requestFrom(uint8_t address);
bool requestFrom(uint8_t address, T &data_struct);
bool requestFrom(uint8_t address, uint8_t *data_pointer, uint8_t data_size);
```

Here:
- `address` is the receivers address in TWI.
- `data_pointer` and `size` can be used to send any buffer.
- `T` is automatically converted to your datatype using templates.

It returns TRUE if the EasyTransfer frame has been received correctly, and FALSE
if there has been an error (e.g., CRC error).

Examples
========

Examples for TWI are included with this package.

- TWI_Master: A TWI master implementation. It send two numbers to a slave.
- TWI_Slave: A TWI slaves implementation. It resend the two numbers to the master to create a Fibonachi series.
- TWI_Slave_External_Buffer: A TWI slaves implementation, with alternative syntax.

Protocol
========

The EasyTransfer protocol is:
   0x06, 0x85, size of payload (uint8_t), Payload, Checksum (in uint8_t)
 The checksum is computed by xoring "size of payload" with each byte of payload

Authors and Contact
===================

 - Farzad Noorian <farzad.noorian@gmail.com>
 - Kwajo Menash <kwajom@gmail.com>

License
=======

This is a complete and clean rewrite of the EasyTransfer protocol
using advanced C++ techniques to facilitate easy porting to different
communication tools, and is distributed under
Apache License, Version 2.0 (the "License");

you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 

