/*
 * Simple Arduino-based program to read values from the HRMI using the I2C interface
 *
 * Connections
 *    Arduino            HRMI
 *    -----------------------
 *      +5                +5 (Power for the HRMI)
 *      GND               GND
 *      Analog In 5       TX (I2C SCL) (recommend 4.7 kOhm pullup)
 *      Analog In 4       RX (I2C SDA) (recommend 4.7 kOhm pullup)
 *
 *
 * Note: By default the Arduino Wiring library is limited to a maximum
 *       I2C read of 32 bytes.  The Get Heartrate command is limited
 *       by this code to a maximum of 30 values (for a max I2C packet
 *       of 32 bytes).
 * 
 */
 
//#include <Time.h>
//#include <DS1302RTC.h>
//#include <OneWire.h>  
#include "Wire.h"
//#include <DallasTemperature.h>  
#include <string.h>  
 
 
//DS18B20 온도 센서의 데이터선인 가운데 핀을 아두이노 5번에 연결합니다.   
//#define ONE_WIRE_BUS 5  


#define HRMI_I2C_ADDR      127
#define HRMI_HR_ALG        1   // 1= average sample, 0 = raw sample
/*
//1-wire 디바이스와 통신하기 위한 준비  
OneWire oneWire(ONE_WIRE_BUS);  
    
// oneWire선언한 것을 sensors 선언시 참조함.  
DallasTemperature sensors(&oneWire);  
    
//다바이스 주소를 저장할 배열 선언  
DeviceAddress insideThermometer;
 

 
 
// Set pins for DS1302 RTC:  CE(reset),IO(dat),CLK
DS1302RTC RTC(2, 3, 4);
 
 */
#include <SoftwareSerial.h>   
     
#define SSID "apple"  //공유기 SSID
#define PASS "12345678"   //공유기 비번
#define DST_IP "192.168.0.3"   //MYSQL 서버 주소 

SoftwareSerial dbgSerial(6, 7); // esp8266의 RX, TX 를 각각 아두이노의 6번,7번핀(변경가능?) dbgserial은 소프트웨어시리얼로 선언된것일뿐임.
//esp8266의 serial port.
 

 
void setup()
{
  setupHeartMonitor(HRMI_HR_ALG);
  
  // Setup Serial connection
  Serial.begin(9600);
 
  //ESP8266
  /////////////////////////////////////////////////////////////////////////  
  //Serial.setTimeout(5000);  
  dbgSerial.begin(9600);   
  Serial.println("ESP8266 connect");  
  
    
   boolean connected=false;  
   for(int i=0;i<10;i++)  
   {  
       if(connectWiFi())  
       {  
         connected = true;  
         break;  
       }  
   }  
     
   if (!connected){while(1);}  
   delay(5000);  
    
   dbgSerial.println("AT+CIPMUX=0");  
 
 
}
 
void loop()
{

	 
  //센서에서 읽어온 심박수를 출력  
  int heartbeat = getHeartRate();
//  int  heartbeat = sensors.getHeartbeat(insideThermometer);  //insideThermometer 의 의미를 파악하자.(파악하지않아도된다)
      
  Serial.print("Current Heartbeat rate: ");  
  Serial.println(heartbeat);
  Serial.println("/min");
 
 
  //DB에 넣기
 
     String cmd = "AT+CIPSTART=\"TCP\",\"";   //AT+CIPSTART	Establish TCP connection or register UDP port
     cmd += DST_IP;  
     cmd += "\",80";  //포트번호 80?	 
     Serial.println(cmd);  
     dbgSerial.println(cmd);  
     if(dbgSerial.find("Error"))  
    {  
      Serial.println( "TCP connect error" );  
      return;  
    }  
       
    char test[20];  
	
    String beat(itoa(heartbeat, test, 10);
	
	//(test,tempC, 2, 0));  
      
     cmd = "GET /run.php?temp='"+beat+"\n\r";  //mySQL db에 값 넣기.
     dbgSerial.print("AT+CIPSEND=");   //AT+CIPSEND	Send data
     dbgSerial.println(cmd.length());  
     Serial.println(cmd);  
       
       
     if(dbgSerial.find(">"))  
     {  
       Serial.print(">");  
       }else  
       {  
         dbgSerial.println("AT+CIPCLOSE");  
         Serial.println("connect timeout");  
         delay(1000);  
         return;  
       }  
       Serial.print(cmd);
       dbgSerial.print(cmd);  
       delay(2000);  
       //Serial.find("+IPD");  
       while (Serial.available())  
       {  
         char c = Serial.read();  
         dbgSerial.write(c);  
         if(c=='\r') dbgSerial.print('\n');  
       }  
       Serial.println("===="); 
 
  
  // Wait one second before repeating :)
  delay (2000);
}
 
 void setupHeartMonitor(int type){
  //setup the heartrate monitor
  Wire.begin();
  writeRegister(HRMI_I2C_ADDR, 0x53, type); // Configure the HRMI with the requested algorithm mode
}

int getHeartRate(){
  //get and return heart rate
  //returns 0 if we couldnt get the heart rate
  byte i2cRspArray[3]; // I2C response array
  i2cRspArray[2] = 0;

  writeRegister(HRMI_I2C_ADDR,  0x47, 0x1); // Request a set of heart rate values 

  if (hrmiGetData(127, 3, i2cRspArray)) {
    return i2cRspArray[2];
  }
  else{
    return 0;
  }
}

void writeRegister(int deviceAddress, byte address, byte val) {
  //I2C command to send data to a specific address on the device
  Wire.beginTransmission(deviceAddress); // start transmission to device 
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

boolean hrmiGetData(byte addr, byte numBytes, byte* dataArray){
  //Get data from heart rate monitor and fill dataArray byte with responce
  //Returns true if it was able to get it, false if not
  Wire.requestFrom(addr, numBytes);
  if (Wire.available()) {

    for (int i=0; i<numBytes; i++){
      dataArray[i] = Wire.read();
    }

    return true;
  }
  else{
    return false;
  }
}

boolean connectWiFi()  
{  
   //dbgSerial.println("AT+CWMODE=1");  
     
   String cmd="AT+CWJAP=\"";   // AT+CWJAP – Connect to AP
   cmd+=SSID;  
   cmd+="\",\"";  
   cmd+=PASS;  
   cmd+="\"";  
   dbgSerial.println(cmd);  
   Serial.println(cmd);  
   delay(3000);  
    
   if(dbgSerial.find("OK"))  
   {  
     Serial.println("OK, Connected to WiFi.");  
     return true;  
   }  
   else  
   {  
     Serial.println("Can not connect to the WiFi.");  
     return false;  
   }  
 } 
