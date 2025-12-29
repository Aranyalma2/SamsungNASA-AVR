#ifndef SAMSUNG_NASA_AVR_H
#define SAMSUNG_NASA_AVR_H

#include <Arduino.h>
#include "NASAProtocol.h"
#include "NASAPacket_AVR.h"

#define NASA_RX_BUFFER_SIZE NASA_MAX_PACKET_SIZE
#define NASA_TX_BUFFER_SIZE NASA_MAX_PACKET_SIZE

typedef void (*PacketHandler)(const NASAPacket& packet);

enum BUS_TYPE :  uint8_t {
    F1_F2 = 0,  // 8N1
    R1_R2 = 1   // 8E1
};

class SamsungNASA {
public:
    SamsungNASA();

    // Initialize the library
    bool begin(
        uint32_t baudRate = 9600,
        int8_t reDePin = -1,
        BUS_TYPE busType = F1_F2,
        uint8_t deviceClass = AddressClass:: Undefined,
        uint8_t deviceChannel = 0,
        uint8_t deviceAddress = 0);
    
    // Stop the library
    void end();
    
    // Set packet handler callback
    void setPacketHandler(PacketHandler handler);
    
    // Must be called regularly to process incoming messages
    void poll();
    
    // Check if a complete packet is available
    bool available();
    
    // Get the last received packet (if available)
    bool getPacket(NASAPacket& packet);
    
    // Send a packet
    bool sendPacket(const NASAPacket& packet);
    
    // Helper:  Send a simple packet
    bool sendPacket(const NASAAddress& destination,
                   uint8_t dataType,
                   uint8_t packetType = PacketType:: Normal);
    
    // Get device address
    const NASAAddress& getDeviceAddress() const { return _deviceAddress; }
    
    // Packet building helper
    NASAPacket createPacket(const NASAAddress& destination, uint8_t dataType);
    
private:
    int8_t _reDePin;
    NASAAddress _deviceAddress;
    PacketHandler _packetHandler;
    uint8_t _packetNumber;
    
    // Receive buffer
    uint8_t _receiveBuffer[NASA_RX_BUFFER_SIZE];
    size_t _receiveBufferPos;
    bool _packetReady;
    
    // Last received packet
    NASAPacket _lastPacket;
    
    // Process receive buffer
    void processReceiveBuffer();
    
    // RS485 control
    void setTransmitMode();
    void setReceiveMode();
};

#endif // SAMSUNG_NASA_AVR_H