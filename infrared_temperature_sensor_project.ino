// Sources:
//  - BLE CODE: https://www.youtube.com/watch?v=UMXS0AfGtYc

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define ANALOG_READ_PIN 2
#define ENABLE_DEBUG false

//uuid's can be generated on https://www.uuidgenerator.net/
#define SERVICE_UUID        "fca9fd2c-cff4-489e-b40c-4c8983c76156"
#define CHARACTERISTIC_UUID "49fb2ad1-7a24-4bd0-abdc-ce80a71a4981"

BLEServer* pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool isDeviceConnected = false;

// This class inherits from BLEServerCallbacks, allowing us to attach custom
// functions to when a device is connected or disconnected.
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    isDeviceConnected = true;
    #if ENABLE_DEBUG
      Serial.println("Device Connected.");
    #endif
    delay(1000);
  }
  void onDisconnect(BLEServer* pServer) {
    isDeviceConnected = false;
    #if ENABLE_DEBUG
      Serial.println("Device Disconnected.");
    #endif
    delay(1000);
  }
};

void setup() {
 // Initialize the serial (USB) communication to the USB
 Serial.begin(115200);
 #if ENABLE_DEBUG
  Serial.println("Serial Comms Up...");
 #endif

 // Create the BLE Device
 BLEDevice::init("ESP32");

 // Create the BLE Server
 pServer = BLEDevice::createServer();
 pServer->setCallbacks(new MyServerCallbacks());

 // Create the BLE Service
 BLEService *pService = pServer->createService(SERVICE_UUID);

 // Create a BLE Characteristic
 pCharacteristic = pService->createCharacteristic(
                     CHARACTERISTIC_UUID,
                     BLECharacteristic::PROPERTY_READ   |
                     BLECharacteristic::PROPERTY_WRITE  |
                     BLECharacteristic::PROPERTY_NOTIFY |
                     BLECharacteristic::PROPERTY_INDICATE
                   );
 pCharacteristic->addDescriptor(new BLE2902());

 // Start the service
 pService->start();

 // Start advertising
 BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
 pAdvertising->addServiceUUID(SERVICE_UUID);
 pAdvertising->setScanResponse(false);
 pAdvertising->setMinPreferred(0x00);
 BLEDevice::startAdvertising();
 #if ENABLE_DEBUG
  Serial.println("Waiting for client connection to notify...");
 #endif
}

void loop() {
  // Advertise BLE if disconnected
  if(!isDeviceConnected) {
    pServer->startAdvertising();
    delay(100); // delay for congestion (phyphox is configured to sample at 10 Hz)
  }

  // Read Voltage from Pin
  float voltage = 3.3*analogRead(ANALOG_READ_PIN)/4095;
  
  // Serial notify change in value
  #if ENABLE_DEBUG
    Serial.println(voltage);
  #endif

  // BLE notify change in value
  if(isDeviceConnected) {
    pCharacteristic->setValue(voltage);
    pCharacteristic->notify();
    delay(100); // delay for congestion (phyphox is configured to sample at 10 Hz)
  }
}
