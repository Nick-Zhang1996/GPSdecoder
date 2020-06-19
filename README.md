GPSdecoder
==========
This is a class-based library for GPS, it can handle a mixture of NMEA 0183 sentences and UBX formatted data.

The class can parse the following NMEA 0183 sentences

GGA (time,lat,lon,fix_quality)
VTG (kph)

Since the default serial rx buffer is only 16 - 64 Bytes and therefore insufficient to contain a complete sentence, you're recommended to modify `HardwareSerial.h` in arduino core to increase the `SERIAL_RX_BUFFER_SIZE`.
