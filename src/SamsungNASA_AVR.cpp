#include "SamsungNASA_AVR.h"

SamsungNASA::SamsungNASA()
    : _reDePin(-1),
      _packetHandler(nullptr),
      _packetNumber(0),
      _receiveBufferPos(0),
      _packetReady(false) {
}

bool SamsungNASA::begin(uint32_t baudRate,
                        int8_t reDePin,
                        BUS_TYPE busType,
                        uint8_t deviceClass,
                        uint8_t deviceChannel,
                        uint8_t deviceAddress) {
    _reDePin = reDePin;
    _deviceAddress = NASAAddress(deviceClass, deviceChannel, deviceAddress);
    
    // Configure RE/DE pin
    if (_reDePin >= 0) {
        pinMode(_reDePin, OUTPUT);
        setReceiveMode();
    }
    
    // Initialize serial
    Serial.begin(baudRate, busType);
    
    return true;
}

void SamsungNASA::end() {
    Serial.end();
}

void SamsungNASA::setPacketHandler(PacketHandler handler) {
    _packetHandler = handler;
}

void SamsungNASA::poll() {
    while (Serial.available()) {
        uint8_t byte = Serial.read();
        
        // Check for start byte
        if (byte == NASA_START_BYTE) {
            _receiveBufferPos = 0;
            _packetReady = false;
        }
        
        // Add to buffer
        if (_receiveBufferPos < NASA_RX_BUFFER_SIZE) {
            _receiveBuffer[_receiveBufferPos++] = byte;
            
            // Check if we have a complete packet
            if (_receiveBufferPos >= 3) {
                size_t expectedSize = (_receiveBuffer[1] << 8) | _receiveBuffer[2];
                size_t totalSize = expectedSize + 2;
                
                if (_receiveBufferPos >= totalSize) {
                    processReceiveBuffer();
                }
            }
        } else {
            // Buffer overflow, reset
            _receiveBufferPos = 0;
            _packetReady = false;
        }
    }
}

bool SamsungNASA::available() {
    return _packetReady;
}

bool SamsungNASA:: getPacket(NASAPacket& packet) {
    if (!_packetReady) {
        return false;
    }
    
    packet = _lastPacket;
    _packetReady = false;
    return true;
}

void SamsungNASA::processReceiveBuffer() {
    if (_lastPacket.decode(_receiveBuffer, _receiveBufferPos)) {
        _packetReady = true;
        
        if (_packetHandler != nullptr) {
            _packetHandler(_lastPacket);
        }
    }
    
    _receiveBufferPos = 0;
}

bool SamsungNASA::sendPacket(const NASAPacket& packet) {
    uint8_t buffer[NASA_TX_BUFFER_SIZE];
    size_t length = const_cast<NASAPacket&>(packet).encode(buffer, NASA_TX_BUFFER_SIZE);
    
    if (length == 0) {
        return false;
    }
    
    // Set transmit mode
    setTransmitMode();
    
    // Small delay for hardware to switch
    delayMicroseconds(50);
    
    // Send data
    Serial.write(buffer, length);
    Serial.flush();
    
    // Set receive mode
    setReceiveMode();
    
    return true;
}

bool SamsungNASA::sendPacket(const NASAAddress& destination,
                             uint8_t dataType,
                             uint8_t packetType) {
    NASAPacket packet = createPacket(destination, dataType);
    packet.getCommand().setPacketType(packetType);
    return sendPacket(packet);
}

NASAPacket SamsungNASA::createPacket(const NASAAddress& destination, uint8_t dataType) {
    NASAPacket packet;
    packet.getSourceAddress() = _deviceAddress;
    packet.getDestinationAddress() = destination;
    packet.getCommand().setDataType(dataType);
    packet.getCommand().setPacketType(PacketType::Normal);
    packet.getCommand().setPacketNumber(_packetNumber++);
    return packet;
}

void SamsungNASA::setTransmitMode() {
    if (_reDePin >= 0) {
        digitalWrite(_reDePin, HIGH);
    }
}

void SamsungNASA:: setReceiveMode() {
    if (_reDePin >= 0) {
        digitalWrite(_reDePin, LOW);
    }
}