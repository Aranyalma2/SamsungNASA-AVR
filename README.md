# SamsungNASA-AVR - Samsung NASA Protocol Library for AVR

An Arduino library for communicating with Samsung HVAC systems using the NASA protocol over RS485. Optimized for AVR microcontrollers. For ESP32 platform check this: [SamsungNASA](https://github.com/Aranyalma2/SamsungNASA)

## Features

- **Full NASA Protocol Implementation** - Complete encoding/decoding based on relativly new (but unknown) version of the protocol
- **AVR Optimized** - OPtimised for low memory environment
- **RS485 Support** - Built-in RE/DE pin control for RS485 transceivers
- **CRC16 Validation** - Automatic packet integrity checking
- **Non-blocking polling** - Polling architecture to process incomming packets
- **All Message Types** - Supports Enum, Variable, LongVariable, and Structure messages
- **Flexible Device Types** - Configure as any NASA protocol device (default: Undefined)

## Hardware Requirements

- **Arduino board** (Uno, Mega, Nano, etc.)
- **RS485 Transceiver** module (MAX485, MAX3485, etc.)
- Connection to **Samsung HVAC** NASA bus

## Wiring

```
RS485 Module          Arduino
─────────────────────────────
RO (Receiver Out) →   RX
DI (Driver In)    →   TX
RE (Recv Enable)  →   Pin 4 (or any digital pin)
DE (Driver Enable)→   Pin 4 (same as RE)
VCC               →   5V (or 3.3V)
GND               →   GND

RS485 A/B         →   Samsung F1/F2 or R1/R2
```

## Protocol Details

### Packet Structure

```
┌────────┬──────┬──────┬────────┬──────────┬─────────┬──────────┬────────┬─────┬────────┐
│ Start  │ Size │ Size │ Source │ Dest.    │ Command │ Msg      │ Data   │ CRC │ End    │
│ (0x32) │ (HI) │ (LO) │ (3B)   │ (3B)     │ (3B)    │ Count    │ (var)  │ (2B)│ (0x34) │
└────────┴──────┴──────┴────────┴──────────┴─────────┴──────────┴────────┴─────┴────────┘
```

### Bus Types

- **F1_F2**: 8N1 (8 data bits, No parity, 1 stop bit) - Most common
- **R1_R2**: 8E1 (8 data bits, Even parity, 1 stop bit)

### Message Types

- **Enum** (Type 0): 1-byte value (e.g., ON/OFF, mode)
- **Variable** (Type 1): 2-byte value (e.g., temperature)
- **LongVariable** (Type 2): 4-byte value (e.g., power)
- **Structure** (Type 3): Variable length data

### Temperature Encoding

Temperatures are encoded as `value * 10`:

- 24.0°C → 240
- 18.5°C → 185

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Credits

Based on the Samsung NASA protocol reverse engineering efforts and few surfaced documents.

Thanks to the contributors of [ESPHome Samsung HVAC Integration](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus) for interpreting some of the protocol addresses.

## Disclaimer

This library is intended for educational use in home and educational applications and is not affiliated with or endorsed by Samsung. Use at your own risk. Always comply with local regulations regarding modifications to HVAC systems.
