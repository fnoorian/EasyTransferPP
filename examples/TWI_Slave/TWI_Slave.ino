// An example of TwoWire Slave EasyTransfer++, implementing Fibonachi
// Master sends a and b to slave
// Master then requests b and a+b from a slave

#include <Wire.h>
#include "ETPP.h"

//define slave i2c addresses
const uint8_t master_address = 16;
const uint8_t slave_1_address = 64;

//define structs
struct request_t {
  int a;
  int b;
};

struct response_t {
  int b;
  int a_plus_b;
};

//setup easyTrasnfer objects, with an internal buffer
EasyTransferPP<TwoWire, request_t> et_wire(Wire);

void setup() {
  Serial.begin(9600);
  
  Wire.begin(slave_1_address);

  // Set callbacks for slave operation
  Wire.onReceive(onreceive);
  Wire.onRequest(onrequest);
}

void loop() {

}

// Callback function for when data is received
void onreceive(int numbytes) {
  bool frameCompleted = et_wire.receive();
  
  if (frameCompleted) {
    // get the results from internal buffer
    const request_t & request = et_wire.receiveBuffer();
    
    Serial.println("data received");
    Serial.print("a: ");
    Serial.println(request.a);
    Serial.print("b: ");
    Serial.println(request.b);
  }  else {
    Serial.println("Error while receiving the ET frame.");
  }
}

// Callback function for when data is requested
void onrequest(void) {
  
  const request_t & request = et_wire.receiveBuffer();
  response_t resp = { request.b, request.a + request.b};
  et_wire.send(resp);
}

