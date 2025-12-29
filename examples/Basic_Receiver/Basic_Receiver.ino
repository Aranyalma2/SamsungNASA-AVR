/*
  Basic Receiver Example for SamsungNASA_AVR Library
  
  This example demonstrates packet reception WITHOUT using callbacks.
  Instead, it manually checks for available packets using available()
  and getPacket() methods.
  
  This approach gives you more control over when packets are processed.
  
  Hardware Requirements:
  - Arduino AVR board (Uno, Mega, Nano, etc.)
  - RS485 module connected to Serial (TX/RX)
  - DE/RE control pin (default: pin 2)
*/

#include <SamsungNASA_AVR.h>

// Create NASA instance
SamsungNASA nasa;

// Configuration
const uint32_t BAUD_RATE = 9600;
const int8_t RE_DE_PIN = 2;
const BUS_TYPE BUS = F1_F2;

// Device address configuration
const uint8_t MY_CLASS = AddressClass::Indoor;
const uint8_t MY_CHANNEL = 0;
const uint8_t MY_ADDRESS = 1;

void setup() {
  // Initialize NASA protocol
  if (!nasa.begin(BAUD_RATE, RE_DE_PIN, BUS, MY_CLASS, MY_CHANNEL, MY_ADDRESS)) {
    debugSerial.println(F("Failed to initialize NASA! "));
    // Failed to initialize - halt with fast blink
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  
  // Configure LED
  pinMode(LED_BUILTIN, OUTPUT);
  
}

void loop() {
  // STEP 1: Always call poll() to process incoming serial data
  nasa.poll();
  
  // STEP 2: Check if a complete packet is available
  if (nasa.available()) {
    // Packet is ready! 
    digitalWrite(LED_BUILTIN, HIGH);
    
    // STEP 3: Get the packet
    NASAPacket packet;
    if (nasa.getPacket(packet)) {
      // STEP 4: Process the packet
      handlePacket(packet);
    }
    
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  // Do other work here...
  // The poll() method is non-blocking
}

void handlePacket(const NASAPacket& packet) {
  // Get command type
  const NASACommand& cmd = packet.getCommand();
  
  // Handle different data types
  switch (cmd.getDataType()) {
    case DataType::Read: 
      handleReadRequest(packet);
      break;
      
    case DataType::Write:
      handleWriteCommand(packet);
      break;
      
    case DataType:: Notification:
      handleNotification(packet);
      break;
      
    case DataType::Response:
      handleResponse(packet);
      break;
  }
}

void handleReadRequest(const NASAPacket& packet) {
  // Someone is requesting data from us
  // Send back a response with current values
  
  NASAAddress destination = packet.getSourceAddress();
  NASAPacket response = nasa.createPacket(destination, DataType::Response);
  
  // Add dummy data
  NASAMessageSet powerMsg(MessageNumber::ENUM_IN_OPERATION_POWER);
  powerMsg.setValue(1);
  response.addMessage(powerMsg);
  
  nasa.sendPacket(response);
}

void handleWriteCommand(const NASAPacket& packet) {
  // Someone is sending us a command
  // Process each message
  
  for (size_t i = 0; i < packet.getMessageCount(); i++) {
    const NASAMessageSet* msg = packet.getMessage(i);
    if (msg) {
      // Act on the command
      if (msg->getMessageNumber() == MessageNumber::ENUM_IN_OPERATION_POWER) {
        bool powerOn = (msg->getValue() != 0);
        // Turn on/off
      }
    }
  }
  
  // Send ACK
  NASAAddress destination = packet.getSourceAddress();
  nasa.sendPacket(destination, DataType::Ack);
}

void handleNotification(const NASAPacket& packet) {
  // Someone is broadcasting information
  // Just store/display it, no response needed
}

void handleResponse(const NASAPacket& packet) {
  // This is a response to our previous read request
  // Extract the data we need
  
  if (packet.hasMessage(MessageNumber::VAR_IN_TEMP_ROOM_F)) {
    uint32_t temp = packet.getMessageValue(MessageNumber::VAR_IN_TEMP_ROOM_F);
    // temp is in 0.1°C units, so divide by 10
    // Use the temperature value... 
  }
}