#include <Arduino.h>

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define DEVICE_NAME "XYZ" // this will shown on BLE receiver

#define BLE_INTERVAL 1000

#define d_flag 1
#define debug(str) if(d_flag) Serial.print(str)
#define debugln(str) if(d_flag) Serial.println(str)

// deep sleep constants
#define uS_TO_mS_FACTOR 1000   /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP  15000        /* Time ESP32 will go to sleep (in ms)-initial value 30s */

bool deviceConnected=false;
/************************** Variables **************************/

// context variables

RTC_DATA_ATTR int TIME_TO_SLEEP = 0 ,  txValue=0;
RTC_DATA_ATTR int ts = 0, cal;
RTC_DATA_ATTR bool success = false, cl;
RTC_DATA_ATTR bool status_on_off = true, st;


char msg[30];

esp_sleep_wakeup_cause_t wakeup_reason;
uint32_t sleep_tm = TIME_TO_SLEEP*uS_TO_mS_FACTOR;
// BLE

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
BLECharacteristic *pCharacteristic;

//int attempt=0;
char sl[7]; //To store all the time positions where slouch is detected

/************************** Functions **************************/


char tracking[]="RP+00.00@+00.00";
void set_ble(){
  
  pCharacteristic->setValue(tracking);
    
    pCharacteristic->notify(); // Send the value to the app!
    Serial.print("*** Sent Value: ");
    Serial.print(tracking);
    Serial.println(" ***");
    debugln("Data Sent!");
}


void store_values(){
  ts=TIME_TO_SLEEP;

  cl=success;
  cal=txValue;
  st=status_on_off;
}


void load_values(){
 TIME_TO_SLEEP=ts;

 success=cl;
 txValue=cal;
 status_on_off=st;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
String x="";
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue= pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.println(rxValue[i]);
        }

        Serial.println();

        // Do stuff based on the command received from the app
        if (rxValue.find("A") != -1) { 
          txValue=1;
          success=false;
          status_on_off=true;
          
        }

        if (rxValue.find("B") != -1)  //OFF
        {
          int intermediate_sleep=120000*uS_TO_mS_FACTOR;
          Serial.println("OFF string sent");
          status_on_off = false;
          //esp_sleep_enable_timer_wakeup(intermediate_sleep);
          store_values();
          delay(100);
          //esp_deep_sleep_start();
        }

        if (rxValue.find("C") != -1)  //ON
        {
          status_on_off = true;
          success=false;
          Serial.println("Now in main loop");
          Serial.println("ON string sent");
          txValue=0;
          esp_sleep_enable_timer_wakeup(0);
          esp_deep_sleep_start();
        }
      }
    }
};



/************************** Setup Code **************************/
void setup() {
  if(d_flag){
    Serial.begin(115200);
  }
  debugln();
  debugln();
 
  // BLE setup
  BLEDevice::init(DEVICE_NAME);
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start advertising
  pService->start();
  pServer->getAdvertising()->start();  
  debugln("Bluetooth Started.");

  // sleep mode code
 // esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_PIN,1); //1 = High, 0 = Low
  
  wakeup_reason = esp_sleep_get_wakeup_cause();

  set_ble();
  delay(3500); 
}

/************************** Loop Code **************************/
void loop() {  

}
