// Sources:
//  - BLE CODE: https://www.youtube.com/watch?v=UMXS0AfGtYc

#include <Adafruit_MLX90614.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define ENABLE_DEBUG true
#define I2C_0_SDA 21
#define I2C_0_SCL 22
#define I2C_0_FREQ_HZ (100##000)

//uuid's can be generated on https://www.uuidgenerator.net/
#define SERVICE_UUID        "fca9fd2c-cff4-489e-b40c-4c8983c76156"
#define CHARACTERISTIC_UUID "49fb2ad1-7a24-4bd0-abdc-ce80a71a4981"

BLEServer* pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool isDeviceConnected = false;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
TwoWire i2c_0 = TwoWire(0);

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

 i2c_0.begin(I2C_0_SDA, I2C_0_SCL, I2C_0_FREQ_HZ);
 if (!mlx.begin(MLX90614_I2CADDR, &i2c_0)) {
  #if ENABLE_DEBUG
    Serial.println("Error connecting to MLX sensor.");
  #endif
  mlx.readEmissivity();
  while (1);
 }
 
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
    delay(80); // delay for congestion (phyphox is configured to sample at 10 Hz)
  }

  // Read object temperature from sensor
  float object_temperature_C = ((float)mlx.readObjectTempC());
  
  // Serial notify change in value
  Serial.println(object_temperature_C);

  // BLE notify change in value
  if(isDeviceConnected) {
    pCharacteristic->setValue(object_temperature_C);
    pCharacteristic->notify();
    delay(80); // delay for congestion (phyphox is configured to sample at 10 Hz)
  }
}
