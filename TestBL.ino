
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <RtcDS3231.h>

RtcDS3231 Rtc;
int MCU_INT = 12;
const int chipSelect = 4;
String command = "";

SoftwareSerial mySerial(7,8);

void setSetting(String setting, int value){
  Serial.println(setting + ": " + value);
}

void executeCommand(String command) {
  if(command == "turnOn"){
     digitalWrite(13, HIGH);
  } else if(command == "turnOff"){
    digitalWrite(13, LOW);
  } else if (command.startsWith("setWaterAmmount")){
    command.remove(0, sizeof("setWaterAmmount=") - 1);
    int value = command.toInt();
    setSetting("waterAmount", value);
  } else if(command == "getData"){
    sendLogData();
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
  SD.begin(chipSelect);
  Rtc.Begin();
}

void serialEvent(){
  while(Serial.available()){
    char input = Serial.read();
    if(input == '!'){
      Serial.println(command);
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

  Serial.println("before while");
  while (dataLog.available()) {
    buffer = dataLog.readStringUntil('\n');
    Serial.println(buffer);      
    Serial.println("in while");
    buffer = "";
  }

  Serial.println("After while");
  SD.remove(filename);
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

    Serial.println(datestring);
    return datestring;
}


// the loop function runs over and over again forever
void loop() {          // wait for a second
  int soilMesurement = getSoilHumidityMesurement();
  saveMesurements(soilMesurement);
  Serial.println(soilMesurement); 
  delay(100000);
}
