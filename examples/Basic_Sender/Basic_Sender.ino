/*
  BasicSender Example for SamsungNASA_AVR Library
  
  This example demonstrates how to send NASA protocol packets
  using the AVR-compatible synchronous library.
  
  Hardware Requirements:
  - Arduino AVR board (Uno, Mega, Nano, etc.)
  - RS485 module connected to Serial (TX/RX)
  - DE/RE control pin (default: pin 2)
  
  Connections:
  - RS485 TX -> Arduino RX (pin 0)
  - RS485 RX -> Arduino TX (pin 1)
  - RS485 DE -> Arduino pin 2
  - RS485 RE -> Arduino pin 2
*/

#include <SamsungNASA_AVR.h>

// Create NASA instance
SamsungNASA nasa;

// Configuration
const uint32_t BAUD_RATE = 9600;
const int8_t RE_DE_PIN = 2;
const BUS_TYPE BUS = F1_F2;  // F1_F2 for 8N1, R1_R2 for 8E1

// Device address configuration
const uint8_t MY_CLASS = AddressClass::Undefined;
const uint8_t MY_CHANNEL = 0;
const uint8_t MY_ADDRESS = 1;

// Target device address
const uint8_t TARGET_CLASS = AddressClass::Indoor;
const uint8_t TARGET_CHANNEL = 0;
const uint8_t TARGET_ADDRESS = 1;

// Timing
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 5000; // Send every 5 seconds

void setup() {
  // Initialize NASA protocol
  if (!nasa.begin(BAUD_RATE, RE_DE_PIN, BUS, MY_CLASS, MY_CHANNEL, MY_ADDRESS)) {
    // Failed to initialize - halt
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  delay(100);
}

void loop() {
  // Process any incoming messages (important even for sender)
  nasa.poll();
  
  // Check if it's time to send
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = currentTime;
    
    // Send a read request
    sendReadRequest();
    
    // Blink LED to indicate transmission
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void sendReadRequest() {
  // Create destination address
  NASAAddress destination(TARGET_CLASS, TARGET_CHANNEL, TARGET_ADDRESS);
  
  // Create packet for reading data
  NASAPacket packet = nasa.createPacket(destination, DataType::Read);
  
  // Add message requests
  // Request current temperature
  NASAMessageSet tempMsg(MessageNumber::VAR_IN_TEMP_ROOM_F);
  packet.addMessage(tempMsg);
  
  // Request power state
  NASAMessageSet powerMsg(MessageNumber:: ENUM_IN_OPERATION_POWER);
  packet.addMessage(powerMsg);
  
  // Request operation mode
  NASAMessageSet modeMsg(MessageNumber:: ENUM_IN_OPERATION_MODE);
  packet.addMessage(modeMsg);
  
  // Send the packet
  if (nasa.sendPacket(packet)) {
    // Packet sent successfully
  } else {
    // Failed to send packet
  }
}

void sendWriteCommand() {
  // Example: Send a write command to turn on the unit
  NASAAddress destination(TARGET_CLASS, TARGET_CHANNEL, TARGET_ADDRESS);
  
  // Create packet for writing data
  NASAPacket packet = nasa.createPacket(destination, DataType::Write);
  
  // Set power ON
  NASAMessageSet powerMsg(MessageNumber::ENUM_IN_OPERATION_POWER);
  powerMsg.setValue(1); // 1 = ON, 0 = OFF
  packet.addMessage(powerMsg);
  
  // Set target temperature (30°C = 0x001E in the protocol)
  NASAMessageSet tempMsg(MessageNumber::VAR_IN_TEMP_TARGET_F);
  tempMsg.setValue(0x001E); // 30°C
  packet.addMessage(tempMsg);
  
  // Send the packet
  nasa.sendPacket(packet);
}

void sendBroadcast() {
  // Example: Send a broadcast message
  NASAAddress broadcast(AddressClass::BroadcastLocalLayer, 0, 0);
  
  // Create packet for notification
  NASAPacket packet = nasa.createPacket(broadcast, DataType::Notification);
  
  // Add notification message
  NASAMessageSet notifyMsg(MessageNumber:: ENUM_IN_OPERATION_POWER);
  notifyMsg.setValue(1);
  packet.addMessage(notifyMsg);
  
  // Send the packet
  nasa.sendPacket(packet);
}