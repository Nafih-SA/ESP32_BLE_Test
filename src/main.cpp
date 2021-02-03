#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "uuid.h"

uint8_t level = 57;
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
// BLECharacteristic **dpCharacteristic = NULL;
// BLECharacteristic *pCharacteristic2  = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

void addService(BLEService *name, BLECharacteristic **ptr, BLEUUID sUUID, BLEUUID cUUID)
{
  name = pServer->createService(sUUID);
  // Create a BLE Characteristic
  *ptr = name->createCharacteristic(
      cUUID, BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  // Create a BLE Descriptor
  (*ptr)->addDescriptor(new BLE2902());

  // Start the service
  name->start();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE Server");

  BLEDevice::init("ESP32_BLE_Test");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  /********** Service 1 **************/
  BLEService *s1;
  // dpCharacteristic = &pCharacteristic;
  addService(s1, &pCharacteristic, SERVICE_UUID, CHARACTERISTIC_UUID);

  /********** Battery Service **************/
  // BLEService *s2;
  // addService(s2, pCharacteristic2, BatteryService, BatteryCharacteristics);
  // BLEService *pService2 = pServer->createService(BatteryService); //Declared Globally
  // BLECharacteristic *pCharacteristicBatt = pService2->createCharacteristic(BatteryChara, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  // pCharacteristicBatt->setValue("Percentage 0 - 100");
  // pCharacteristicBatt->addDescriptor(&BatteryLevelDescriptor);
  // pCharacteristicBatt->addDescriptor(new BLE2902());
  // pService2->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop()
{

  if (deviceConnected)
  {
    pCharacteristic->setValue((uint8_t *)&value, 4);
    pCharacteristic->notify();
    value++;
    delay(100); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  delay(1000);
}