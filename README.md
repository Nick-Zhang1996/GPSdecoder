GPSdecoder
==========
This is a class-based library for GPS, it parses NEMA 0183 formatted GPS data.


feature added: automatically read from SoftwareSerial

Written on: Xcode
Target platform:Arduino
Language:C++


Currently implemented sentence types are:
Note: duplicated data is only implemented once in order to save computing source

RMC(full feature)
GGA(DGPS,GPS position not implemented)
GSA(partially supported)
VTG(partially supported)

other sentences will not be supported, because my GPS doesn’t feature them :)
contact me if you find a bug or have and need or advice


