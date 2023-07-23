


#include "arduino.h"

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>

extern bool reply;
extern TinyGsm modem;
extern TinyGsmClient client;

#define SerialAT		Serial1
#define SerialDEBUG		Serial

// Modem connections to the ESP32
#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

// SD Card Interface
#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define LED_G1		12

const char oper[]  = "MEO";						// Mobile Network Provider
//const char oper[]  = "vodafone P";

// GPRS credentials
const char apn[]      = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

bool reply = false;
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);


void setup() {
	
	// Sets the serial channels (DEBUG and GSM modem communications)
	SerialDEBUG.begin(UART_BAUD);
	SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
	delay(100);

	// Model Information
	SerialDEBUG.println();
	SerialDEBUG.println("========================================");
	SerialDEBUG.println("*** LILYGO-T-SIM7000G ESP32 ***");
	SerialDEBUG.printf("ESP32 Model %s with %dx cores at %dMHz\n",ESP.getChipModel(),ESP.getChipCores(),ESP.getCpuFreqMHz());
	SerialDEBUG.println("Firmware v0.0");
	SerialDEBUG.println("========================================");
	SerialDEBUG.println();
	
	// Memory Information
	SerialDEBUG.println("========================================");
	SerialDEBUG.printf("Flash Memory: %d, at %d MHz \n",ESP.getFlashChipSize(),ESP.getFlashChipSpeed()/1000000);
	SerialDEBUG.println("========================================");
	SerialDEBUG.printf("PSRAM total size     : %u \n", esp_spiram_get_size());
	SerialDEBUG.println("----------------------------------------");
	SerialDEBUG.printf("PSRAM first 4MB size : %u \n", ESP.getPsramSize());
	SerialDEBUG.printf("PSRAM first 4MB free : %u \n", ESP.getMaxAllocPsram());
	SerialDEBUG.println("========================================");
	SerialDEBUG.printf("Internal RAM  size   : %u \n", ESP.getHeapSize());
	SerialDEBUG.printf("Internal RAM  free   : %u \n", ESP.getFreeHeap());
	SerialDEBUG.println("========================================");
	SerialDEBUG.println();
	
	// User LED Test
	pinMode(LED_G1, OUTPUT);
	SerialDEBUG.print("User LED blinking (5x) ...");
	for (char i=0; i<10; i++){
		digitalWrite(LED_G1, i&0x01);
		delay(500);
		SerialDEBUG.print(".");
	}
	digitalWrite(LED_G1, HIGH);						// LED off
	SerialDEBUG.println(" OK");
	
	// Initializes the GSM modem
	SerialDEBUG.println();
	// Sets up the modem power pin
	SerialDEBUG.print("Powering GSM Modem ... ");
	pinMode(PWR_PIN, OUTPUT);
	digitalWrite(PWR_PIN, HIGH);
	delay(300);
	digitalWrite(PWR_PIN, LOW);
	delay(10000);										//Wait for the SIM7000 to set up (10s delay)
	SerialDEBUG.println("OK");

	// Check the modem response and availability
	int i = 10;
	Serial.print("Testing Modem Response ...");
	while (i) {											// 10x attempts with 500ms delay
		SerialAT.println("AT");
		delay(500);
		if (SerialAT.available()) {
			String r = SerialAT.readString();
			if ( r.indexOf("OK") >= 0 ) {
				reply = true;
				break;;
			}
		}
		SerialDEBUG.print(".");
		delay(500);
		i--;
	}
	SerialDEBUG.println(" OK");
	
	// Resets the ESP32 if the modem was not initialized
	if (!reply) {
		SerialDEBUG.println("GSM modem initialization ERROR");
		SerialDEBUG.println("Restarting in 5s");
		delay(5000);
		ESP.restart();
	}
	
	// Prints Modem Information Data
	String dataString = modem.getModemInfo();
	SerialDEBUG.print("Modem Info: ");
	SerialDEBUG.println(dataString);
	
	// Manual Network Registration (AT+COPS=1,1,"MEO")
	SerialDEBUG.print("Connecting to " + String(oper) + " ... ");
	modem.sendAT("+COPS=1,1,\"" + String(oper) + "\"");
	if (modem.waitResponse(10000L) != 1) {
		SerialDEBUG.println("ERROR");
		SerialDEBUG.println("Restarting in 5s");
		delay(5000);
		ESP.restart();
	}
	SerialDEBUG.println("OK");
	
	// Signal Quality Information
	SerialDEBUG.print("Signal quality:");
	SerialDEBUG.println(modem.getSignalQuality());
	
	// GPRS Connection
	SerialDEBUG.print(F("Connecting to "));
	SerialDEBUG.print(apn);
	SerialDEBUG.print(" ...");
	if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
		SerialDEBUG.println(" ERROR");
		SerialDEBUG.println("Restarting in 5s");
		delay(5000);
		ESP.restart();
	}
	SerialDEBUG.println(" OK");
	
	// Validates GPRS Connection
	if (modem.isGprsConnected()) {
		SerialDEBUG.println("GPRS connected");
	}

	// Entering AT Command MODE
	SerialDEBUG.println("AT command mode");
	SerialDEBUG.println();
}

void loop() {


	if (SerialAT.available()) {
		SerialDEBUG.write(SerialAT.read());
	}
	if (SerialDEBUG.available()) {
		char buf = SerialDEBUG.read();
		SerialAT.write(buf);
		SerialDEBUG.write(buf);
	}
	delay(1);

}