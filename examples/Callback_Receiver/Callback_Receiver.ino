/*
  CallbackReceiver Example for SamsungNASA_AVR Library
  
  This example demonstrates how to receive and process NASA protocol packets
  using the AVR-compatible synchronous polling library with custom callback function.
  
  Hardware Requirements:
  - Arduino AVR board (Uno, Mega, Nano, etc.)
  - RS485 module connected to Serial (TX/RX)
  - DE/RE control pin (default: pin 2)
  
  Connections:
  - RS485 TX -> Arduino RX (pin 0)
  - RS485 RX -> Arduino TX (pin 1)
  - RS485 DE -> Arduino pin 2
  - RS485 RE -> Arduino pin 2
  
  Note: For debugging, you can use SoftwareSerial for debug output
  since Serial is used for NASA communication.
*/

#include <SamsungNASA_AVR.h>
#include <SoftwareSerial.h>

// Debug serial (optional)
SoftwareSerial debugSerial(10, 11); // RX, TX

// Create NASA instance
SamsungNASA nasa;

// Configuration
const uint32_t BAUD_RATE = 9600;
const int8_t RE_DE_PIN = 2;
const BUS_TYPE BUS = F1_F2;  // F1_F2 for 8N1, R1_R2 for 8E1

// Device address configuration
const uint8_t MY_CLASS = AddressClass::Indoor;
const uint8_t MY_CHANNEL = 0;
const uint8_t MY_ADDRESS = 1;

// Packet counter
unsigned long packetsReceived = 0;

void setup() {
  // Initialize debug serial (optional)
  debugSerial.begin(9600);
  debugSerial.println(F("NASA Receiver Starting..."));
  
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
  
  debugSerial.println(F("NASA initialized successfully"));
  debugSerial.print(F("Device Address: "));
  debugSerial.print(MY_CLASS, HEX);
  debugSerial.print(F(". "));
  debugSerial.print(MY_CHANNEL);
  debugSerial.print(F("."));
  debugSerial.println(MY_ADDRESS);
  
  // Set packet handler callback (optional - can also use available() and getPacket())
  nasa.setPacketHandler(onPacketReceived);
  
  debugSerial.println(F("Waiting for packets..."));
}

void loop() {
  // IMPORTANT: Must call poll() regularly to process incoming data
  nasa.poll();
  
}

// Packet handler callback
void onPacketReceived(const NASAPacket& packet) {
  packetsReceived++;
  
  debugSerial.println(F("==== Packet Received ===="));
  debugSerial.print(F("Count: "));
  debugSerial.println(packetsReceived);
  
  // Process the packet
  processPacket(packet);
  
  debugSerial.println(F("========================"));
  debugSerial.println();

}

void processPacket(const NASAPacket& packet) {
  // Get source address
  const NASAAddress& source = packet.getSourceAddress();
  debugSerial.print(F("From: "));
  debugSerial.print(source.getClass(), HEX);
  debugSerial.print(F("."));
  debugSerial.print(source.getChannel());
  debugSerial.print(F("."));
  debugSerial.println(source.getAddress());
  
  // Get destination address
  const NASAAddress& dest = packet.getDestinationAddress();
  debugSerial.print(F("To: "));
  debugSerial.print(dest.getClass(), HEX);
  debugSerial.print(F("."));
  debugSerial.print(dest.getChannel());
  debugSerial.print(F("."));
  debugSerial.println(dest.getAddress());
  
  // Get command info
  const NASACommand& cmd = packet.getCommand();
  debugSerial.print(F("Type: "));
  printDataType(cmd.getDataType());
  debugSerial.print(F(" PacketType: "));
  debugSerial.print(cmd.getPacketType());
  debugSerial.print(F(" #"));
  debugSerial.println(cmd.getPacketNumber());
  
  // Process messages
  size_t msgCount = packet.getMessageCount();
  debugSerial.print(F("Messages: "));
  debugSerial.println(msgCount);
  
  for (size_t i = 0; i < msgCount; i++) {
    const NASAMessageSet* msg = packet.getMessage(i);
    if (msg) {
      debugSerial.print(F("  ["));
      debugSerial.print(i);
      debugSerial.print(F("] Msg#"));
      debugSerial.print(msg->getMessageNumber(), HEX);
      debugSerial.print(F(" = "));
      debugSerial. print(msg->getValue());
      debugSerial.print(F(" (0x"));
      debugSerial. print(msg->getValue(), HEX);
      debugSerial. println(F(")"));
      
      // Decode specific messages
      decodeMessage(msg->getMessageNumber(), msg->getValue());
    }
  }
  
  // Check if this is a Read request - send response
  if (cmd.getDataType() == DataType::Read) {
    debugSerial.println(F("Read request detected - sending response"));
    sendResponse(packet);
  }
}

void decodeMessage(uint16_t messageNumber, uint32_t value) {
  switch (messageNumber) {
    case MessageNumber::ENUM_IN_OPERATION_POWER:
      debugSerial.print(F("    Power: "));
      debugSerial.println(value ?  F("ON") : F("OFF"));
      break;
      
    case MessageNumber::ENUM_IN_OPERATION_MODE: 
      debugSerial.print(F("    Mode: "));
      switch (value) {
        case 0: debugSerial.println(F("Auto")); break;
        case 1: debugSerial.println(F("Cool")); break;
        case 2: debugSerial.println(F("Dry")); break;
        case 3: debugSerial.println(F("Fan")); break;
        case 4: debugSerial.println(F("Heat")); break;
        default: debugSerial.println(value); break;
      }
      break;
      
    case MessageNumber::ENUM_IN_FAN_MODE:
      debugSerial.print(F("    Fan:  "));
      switch (value) {
        case 0: debugSerial.println(F("Auto")); break;
        case 1: debugSerial.println(F("Low")); break;
        case 2: debugSerial.println(F("Medium")); break;
        case 3: debugSerial.println(F("High")); break;
        default: debugSerial.println(value); break;
      }
      break;
      
    case MessageNumber::VAR_IN_TEMP_ROOM_F:
      debugSerial.print(F("    Room Temp: "));
      debugSerial.print(value / 10.0);
      debugSerial.println(F("°C"));
      break;
      
    case MessageNumber::VAR_IN_TEMP_TARGET_F:
      debugSerial.print(F("    Target Temp: "));
      debugSerial.print(value / 10.0);
      debugSerial.println(F("°C"));
      break;
      
    case MessageNumber::ENUM_IN_STATE_HUMIDITY_PERCENT:
      debugSerial.print(F("    Humidity: "));
      debugSerial.print(value);
      debugSerial.println(F("%"));
      break;
  }
}

void printDataType(uint8_t dataType) {
  switch (dataType) {
    case DataType::Undefined: debugSerial.print(F("Undefined")); break;
    case DataType::Read: debugSerial.print(F("Read")); break;
    case DataType::Write: debugSerial.print(F("Write")); break;
    case DataType::Request: debugSerial.print(F("Request")); break;
    case DataType::Notification: debugSerial.print(F("Notification")); break;
    case DataType::Response: debugSerial.print(F("Response")); break;
    case DataType::Ack: debugSerial.print(F("Ack")); break;
    case DataType::Nack: debugSerial.print(F("Nack")); break;
    default: debugSerial.print(dataType); break;
  }
}

void sendResponse(const NASAPacket& requestPacket) {
  // Create response packet
  NASAAddress destination = requestPacket.getSourceAddress();
  NASAPacket response = nasa.createPacket(destination, DataType::Response);
  
  // Add response data for each requested message
  for (size_t i = 0; i < requestPacket.getMessageCount(); i++) {
    const NASAMessageSet* reqMsg = requestPacket.getMessage(i);
    if (reqMsg) {
      NASAMessageSet respMsg(reqMsg->getMessageNumber());
      
      // Provide dummy data based on message number
      switch (reqMsg->getMessageNumber()) {
        case MessageNumber::ENUM_IN_OPERATION_POWER:
          respMsg.setValue(1); // ON
          break;
          
        case MessageNumber::ENUM_IN_OPERATION_MODE: 
          respMsg.setValue(1); // Cool
          break;
          
        case MessageNumber::VAR_IN_TEMP_ROOM_F:
          respMsg.setValue(300); // 30.0°C
          break;
          
        case MessageNumber::VAR_IN_TEMP_TARGET_F:
          respMsg.setValue(350); // 35.0°C
          break;
          
        case MessageNumber::ENUM_IN_STATE_HUMIDITY_PERCENT:
          respMsg.setValue(45); // 45%
          break;
          
        default:
          respMsg.setValue(0);
          break;
      }
      
      response.addMessage(respMsg);
    }
  }
  
  // Send response
  if (nasa.sendPacket(response)) {
    debugSerial.println(F("Response sent successfully"));
  } else {
    debugSerial.println(F("Failed to send response"));
  }
}