// An example of TwoWire Master EasyTransfer++, implementing Fibonachi
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

//setup easyTrasnfer objects
EasyTransferPP<TwoWire> commandWire(Wire);

void setup() {
  Serial.begin(9600);
  
  Wire.begin(master_address);

  // Initial request with a=1, b=1
  request_t request = {1, 1};
  commandWire.sendTo(slave_1_address, request);
}

void loop() {

  delay(300); // short delay to allow the slave to send back data

  // Read from slave 1
  response_t response;
  bool frameCompleted = commandWire.requestFrom(slave_1_address, response);
  
  if (frameCompleted) {
    Serial.println("data received");
    Serial.print("b: ");
    Serial.println(response.b);
    Serial.print("a+b: ");
    Serial.println(response.a_plus_b);
  } else {
    Serial.println("Error in receiving the frame");
  }

  //send new data to slave 1
  Serial.println("sending data");

  request_t request;
  request.a = response.b;
  request.b = response.a_plus_b;
  commandWire.sendTo(slave_1_address, request);
}

