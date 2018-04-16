/*
Autonomous DLG
Downlink Teensy

Summary: Parses downlinked S.Port data into UART.

iskushan, 04152018

Versions:
	1.0: Initial release. 04152018.

FrSky S-Port Telemetry Decoder library example
(c) Pawelsky 20160919
Not for commercial use

Note that you need Teensy 3.x/LC or 328P based (e.g. Pro Mini, Nano, Uno) board and FrSkySportDecoder library for this example to work
*/

// Uncomment the #define below to enable internal polling of data.
// Use only when there is no device in the S.Port chain (e.g. S.Port capable FrSky receiver) that normally polls the data.
//#define POLLING_ENABLED

#include "FrSkySportSensor.h"
#include "FrSkySportSensorAss.h"
#include "FrSkySportSensorFcs.h"
#include "FrSkySportSensorFlvss.h"
#include "FrSkySportSensorGps.h"
#include "FrSkySportSensorRpm.h"
#include "FrSkySportSensorSp2uart.h"
#include "FrSkySportSensorVario.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportDecoder.h"
#if !defined(__MK20DX128__) && !defined(__MK20DX256__) && !defined(__MKL26Z64__) && !defined(__MK66FX1M0__) && !defined(__MK64FX512__)
#include "SoftwareSerial.h"
#endif

FrSkySportSensorAss ass;                               // Create ASS sensor with default ID
FrSkySportSensorFcs fcs;                               // Create FCS-40A sensor with default ID (use ID8 for FCS-150A)
FrSkySportSensorFlvss flvss1;                          // Create FLVSS sensor with default ID
FrSkySportSensorFlvss flvss2(FrSkySportSensor::ID15);  // Create FLVSS sensor with given ID
FrSkySportSensorGps gps;                               // Create GPS sensor with default ID
FrSkySportSensorRpm rpm;                               // Create RPM sensor with default ID
FrSkySportSensorSp2uart sp2uart;                       // Create SP2UART Type B sensor with default ID
FrSkySportSensorVario vario;                           // Create Variometer sensor with default ID
#ifdef POLLING_ENABLED
FrSkySportDecoder decoder(true);                     // Create decoder object with polling
#else
FrSkySportDecoder decoder;                           // Create decoder object without polling
#endif

uint32_t currentTime, displayTime;
uint16_t decodeResult;
uint8_t validDecode = 0;

void setup()
{
	// Configure the decoder serial port and sensors (remember to use & to specify a pointer to sensor)
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK66FX1M0__) || defined(__MK64FX512__)
	decoder.begin(FrSkySportSingleWireSerial::SERIAL_3, &ass, &fcs, &flvss1, &flvss2, &gps, &rpm, &sp2uart, &vario);
#else
	decoder.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_12, &ass, &fcs, &flvss1, &flvss2, &gps, &rpm, &sp2uart, &vario);
#endif
	Serial.begin(115200);
}

void loop()
{
	// Read and decode the telemetry data, note that the data will only be decoded for sensors
	// that that have been passed to the begin method. Print the AppID of the decoded data.
	decodeResult = decoder.decode();
	if (decodeResult != SENSOR_NO_DATA_ID) {
		//Serial.print("Decoded data with AppID 0x");
		//Serial.println(decodeResult, HEX); 
		validDecode = 1;
	}

	// Display data once a second to not interfeere with data decoding
	currentTime = millis();
	if (currentTime > displayTime)
	{
		displayTime = currentTime + 1000;

		Serial.println("");

		//print valid decode flag
		Serial.print(validDecode);
		Serial.print(",");
		//print date and time
		char dateTimeStr[18];
		sprintf(dateTimeStr, "%02u%02u%04u,%02u%02u%02u", gps.getDay(), gps.getMonth(), gps.getYear() + 2000, gps.getHour(), gps.getMinute(), gps.getSecond());
		Serial.print(dateTimeStr); // Date (year - need to add 2000, month, day) and time (hour, minute, second)
		
		//print GPS latitude,longtitude,altitude, and course
		Serial.print(","); Serial.print(gps.getLat(), 6);
		Serial.print(","); Serial.print(gps.getLon(), 6);		// Latitude and longitude in degrees decimal (positive for N/E, negative for S/W)
		Serial.print(","); Serial.print(gps.getAltitude());		// Altitude in m (can be negative)
		Serial.print(","); Serial.print(gps.getSpeed());		// Speed in m/s
		Serial.print(","); Serial.print(gps.getCog());			// Course over ground in degrees (0-359, 0 = north)

		// Get variometer data (altitude in m, VSI in m/s)
		Serial.print(","); Serial.print(vario.getAltitude());   // Altitude in meters (can be negative)
		Serial.print(","); Serial.print(vario.getVsi());        // Vertical speed in m/s (positive - up, negative - down)

		// Get airspeed sensor (ASS) data
		Serial.print(","); Serial.print(ass.getSpeed());		// Airspeed in km/h

		// Get temperature
		Serial.print(","); Serial.print(rpm.getT1());

		// Get current/voltage sensor (FCS) data
		Serial.print(","); Serial.print(fcs.getCurrent());		// Current consumption in amps
		Serial.print(","); Serial.print(fcs.getVoltage());		// Battery voltage in volts

		validDecode = 0;										//reset the valid decode flag
		Serial.println("");
	}
}