
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <RtcDS3231.h>
#include <avr/pgmspace.h>

// ID of the settings block
#define CONFIG_VERSION "ls1"
#define SETTING_PREFIX "set_setting:"

RtcDS3231 Rtc;
int MCU_INT = 12;
const int chipSelect = 4;
int waterPump = 6;
String command = "";
struct SettingsStruct {
    char version[4];
    int water, humidity;
} settingsStorage = { 
    CONFIG_VERSION,
    5000, 40
};

SoftwareSerial mySerial(7,8);

void executeCommand(String command) {
  if(command == "turnOn"){
    waterPlant();
  } else if (command.startsWith(SETTING_PREFIX)){
    command.remove(0, sizeof(SETTING_PREFIX) - 1);
    setSetting(command);
  } else if(command == "getData"){
    sendLogData();
  } else if(command == "getSettings"){
    sendSettings();
  }
}

void setSetting(String command){
  String settingName;
  int value;

  if(command.startsWith("setWaterAmmount")){
    command.remove(0, sizeof("setWaterAmmount=") - 1);
    value = command.toInt();
    settingsStorage.water = value;
  } else {
    command.remove(0, sizeof("setSoilHumidity=") - 1);
    value = command.toInt();
    settingsStorage.humidity = value;
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, LOW);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
  SD.begin(chipSelect);
  Rtc.Begin();
}

void serialEvent(){
  while(Serial.available()){
    char input = Serial.read();
    if(input == '!'){
      executeCommand(command);
      command = "";
    } else{
      command = command + String(input);
    }
  }
}

int getSoilHumidityMesurement(){
   digitalWrite(9, HIGH);
   delay(1000);
   int humidity = analogRead(A3);
   digitalWrite(9, LOW);

   return humidity;
}

void saveMesurements(int soilMesurement){
  String time = getDateTime();
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
   if (dataFile) {
    dataFile.println(String(soilMesurement) + "#" + time);
    dataFile.close();
   }
}

void sendLogData(){
  String buffer;
  File dataLog = SD.open("datalog.txt");

  Serial.print("data@");
  while (dataLog.available()) {
    buffer = dataLog.readStringUntil('\n');
    Serial.print(buffer);      
    buffer = "";
    if(dataLog.available()){
      Serial.print(",");
    }
  }
  Serial.print("!");
  
  SD.remove("datalog.txt");
}

void sendSettings(){
  Serial.print("settings@" + String(settingsStorage.water) + "#" + String(settingsStorage.humidity) + "!");
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

String getDateTime()
{
    RtcDateTime now = Rtc.GetDateTime();
   char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            now.Month(),
            now.Day(),
            now.Year(),
            now.Hour(),
            now.Minute(),
            now.Second() );

    return datestring;
}

void waterPlant(){
  digitalWrite(waterPump, HIGH);
  delay(settingsStorage.water);
  digitalWrite(waterPump, LOW);
}


// the loop function runs over and over again forever
void loop() {          // wait for a second
  int soilMesurement = getSoilHumidityMesurement();
  int humidityMesurementTreshold = 1023 - (settingsStorage.humidity*10.23);
  Serial.println(humidityMesurementTreshold);
  Serial.println(soilMesurement);
  if(soilMesurement >= humidityMesurementTreshold){
    waterPlant();
  }
  saveMesurements(soilMesurement);
  delay(1000);
}
