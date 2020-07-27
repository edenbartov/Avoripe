

#include "Config.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] = AIO_USERNAME;
const char MQTT_PASSWORD[] = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);/****************************** Feeds ***************************************/

// Setup feeds for temperature & humidity
const char PRESSURE_FEED[] = AIO_USERNAME "/feeds/avocado.squishiness";
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, PRESSURE_FEED);

const char COLOR_FEED[] = AIO_USERNAME "/feeds/avocado.color";
Adafruit_MQTT_Publish avocadoColor = Adafruit_MQTT_Publish(&mqtt, COLOR_FEED);

const char RIPE_FEED[] = AIO_USERNAME "/feeds/avocado.isripe";
Adafruit_MQTT_Publish avocadoRipe = Adafruit_MQTT_Publish(&mqtt, RIPE_FEED);
////////////////////////////////////////////

//// initiate variables //// 
float preasure = 0; //sets the initial preasure
int frequency = 0; // sets the initial light frequency
int angle = 0;
int comfyAngle = 0;
bool isComfy =false;
int isRipe = 1; // if ripe = 2.  not ripe = 1
int state = 0; // 1 for chack, 0 for not to check
// color variables
int red = 0;
int green = 0;
int blue = 0;
Servo myServo; //define the servo
////////////////////////////

// //Calibrated value for color sensor////
int cal_min = 5;
int cal_max_r = 50;
int cal_max_g = 50;
int cal_max_b = 50;
////////////////////////////


////// setting up BLYNK///////
//Blynk authorization code - Make sure you change it 
char auth[] = "";
////////////////////////////

//////////////////////////
/////////setup////////////
//////////////////////////
void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(out, INPUT);
  //myServo.attach (SERVOPIN);

//attachInterrupt(digitalPinToInterrupt(out), Ch1Change, CHANGE);

  // Setting frequency-scaling to 20%
  //light sensor pins S0 and S1 are responsible for 
  digitalWrite(S0,HIGH);
  digitalWrite(S1,HIGH);
  myServo.write(0);
  Serial.begin(9600);

////set up the blynk////
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSPHRASE);
}



//////////////////////////
/////////loop/////////////
//////////////////////////
void loop() {
  Blynk.run(); // run blynk
  //Serial.println(readColor());
  char opp = Serial.read();
  if (opp == 'c') {
    calibrate();
   }

//////////////////////////
/////////functions////////
//////////////////////////  
}

BLYNK_WRITE(V0){ 
   int pinValue = param.asInt(); //get the value of the virtual pin
  if (pinValue == 1) { // press the avocado
      myServo.attach (SERVOPIN);
      Serial.println("Checking");
      CheckAvocado();
      Serial.print("isRipe: ");
      Serial.println(isRipe);
      if(isRipe==2){
      Blynk.notify("Great news! your avocado is ripe!");
      Blynk.virtualWrite(4, isRipe);
      }
  }else{
      Blynk.virtualWrite(4, 1);
      myServo.write(0);
      myServo.detach();

  }
}


//////avocado check and servo functions ///////

int CheckAvocado(){
  
  preasure = getPress(); //reads the preasure
  String temp = readColor();
  char avoColor[] = {};
  temp.toCharArray(avoColor,8);
      Serial.println(avoColor);

  if(!isComfy){
    comfyAngle = gettingComfy();
  }
  delay(1000);
   if(checkRipe(comfyAngle)){ //is the avocado ripe
      isRipe = 2; 
   }else{
    isRipe = 1; 
   }
    Serial.println("try to publish!");
    // reconnect to adafruit IO
    if (! mqtt.connected())
      connect();

  // Publish data to adafuit IO
  if (! pressure.publish(preasure))
    Serial.println("Failed to publish pressure");
  else
    Serial.println("Pressure published!");
    
  if (! avocadoColor.publish(avoColor))
    Serial.println("Failed to publish Color");
  else
    Serial.println("Color published!");
    
  if (! avocadoRipe.publish(isRipe))
    Serial.println("Failed to publish ripness");
  else
    Serial.println("ripness published!");
      

   //////reset/////
    preasure = 0; //sets the initial preasure
    frequency = 0; // sets the initial light frequency
    angle = 0;
    comfyAngle = 0;
    isComfy =false;
    myServo.write(angle);
   /////


  
}
//returning the level of force on the sensor after an avrage of PRESSNUM measurement  
float getPress() {
  delay(100);
  float avg = 0;
  for(int i = 0;i<PRESSNUM;i++){
    avg+= analogRead(preasurePin); //reads the preasure
    yield;
  }
  return avg/PRESSNUM;
}


//geting the avocado cozy and rerurning the the comfy angle
int gettingComfy(){
  while(preasure<23 && angle <100){
    Serial.print("Comfy preasure"); //prints the Preasure sensor reading
    Serial.println(preasure); //prints the Preasure sensor reading
    
    // advance the servo 5 degrees
    angle+=5; 
    myServo.write(angle);
    
    delay(100);
    preasure = getPress();
  }
  isComfy = true;
  return angle;

}

bool checkRipe(int comfyAngle){
    int oldPreasure = preasure;
    Serial.print("comfyAngle "); 
    Serial.println(comfyAngle); //prints the Preasure sensor reading
    myServo.write(angle + 25);  // change the servo angle to squeeze harder on the avocado (by 25 degrees)
    Serial.print("ripe angle"); 
    Serial.println(angle +25); //prints the angle + 25 reading
    Serial.print("ripe preasure"); 
    preasure = getPress(); 
    Serial.println(preasure); //prints the Preasure sensor reading

    //checks if the oldPreasure is smaller then preasure- FORCE if so then the avocado is ripe
    return (preasure < oldPreasure + FORCE); //reads the preasure

}
//////End avocado check and servo functions ///////

////// Colors functions ///////
String readColor() {

  String returned= "#";

  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);

  frequency = pulseIn(out, LOW);  // Reading the output frequency

  frequency = map(frequency, cal_min,cal_max_r,255,0);  //Remaping the value of the frequency to the RGB Model of 0 to 255 for RED
    if (frequency <0){
    frequency =0;
  }
returned += addToString(frequency);
  
  // Printing the value on the serial monitor
  Serial.print("R= ");//printing name
  Serial.print(frequency);//printing RED color frequency
  Serial.print("  "); 
  delay(100);

  
  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);

  
  frequency = pulseIn(out, LOW);  // Reading the output frequency

  frequency = map(frequency, cal_min,cal_max_g,255,0);   // Printing the value on the serial monitor
    if (frequency <0){
    frequency =0;
  }
  
returned += addToString(frequency);


  Serial.print("G= ");//printing name
  Serial.print(frequency);//printing GREEN color frequency
  Serial.print("  ");
  delay(100);

  
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  frequency = pulseIn(out, LOW);  // Reading the output frequency

  frequency = map(frequency, cal_min,cal_max_b,255,0);  //Remaping the value of the frequency to the RGB Model of 0 to 255 for BLUE
  if (frequency <0){
    frequency =0;
  }

returned += addToString(frequency);

  // Printing the value on the serial monitor
  Serial.print("B= ");//printing name
  Serial.print(frequency);//printing BLUE color frequency
  Serial.println("  ");
  delay(100);


Serial.println (returned);
return returned;

}

String addToString (int frequency){
  String str ="";
  if (frequency < 16){
  str+="0";
  str+=String(frequency,HEX);
  }
  else{
  str+=String(frequency,HEX);
  }
  return str;
}
  
  void calibrate() {
    Serial.println("Clear sensor area. Then enter c again");
    interrupts();
      while (Serial.read() != 'c') {
        //do nothing
        yield;
      }
      color();
      cal_max_r = red;
      cal_max_g = green;
      cal_max_b = blue;
      Serial.println("Put white color infront of sensor, Then enter c again");
      while (Serial.read() != 'c') {
        //do nothing
        delay(10);
        yield;
      }
      color();
      cal_min = (red + green + blue) / 3;
      Serial.println("calibrated successfully.");
      delay(300);
  }
   
  void color() {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    //count OUT, pRed, RED
    red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
    digitalWrite(S3, HIGH);
    //count OUT, pBLUE, BLUE
    blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
    digitalWrite(S2, HIGH);
    //count OUT, pGreen, GREEN
    green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  }
   
 

  void remap(){
    red = map(red, cal_min, cal_max_r, 255, 0);
    green = map(green, cal_min, cal_max_g, 255, 0);
    blue = map(blue, cal_min, cal_max_b, 255, 0);
    Serial.print("R Intensity:");
    Serial.print(red);
    Serial.print(" G Intensity: ");
    Serial.print(green);
    Serial.print(" B Intensity : ");
    Serial.println(blue);
    delay(200);
  }

//////End of Colors functions ///////

// connect to adafruit io via MQTT
void connect() {
  Serial.print("Connecting to Adafruit IO... ");
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println("Wrong protocol"); break;
      case 2: Serial.println("ID rejected"); break;
      case 3: Serial.println("Server unavail"); break;
      case 4: Serial.println("Bad user/pass"); break;
      case 5: Serial.println("Not authed"); break;
      case 6: Serial.println("Failed to subscribe"); break;
      default: Serial.println("Connection failed"); break;
    }
    if (ret >= 0) {
      mqtt.disconnect();
    }
    Serial.println("Retrying connection...");
    delay(5000);
  }
  Serial.println("Adafruit IO Connected!");
}
