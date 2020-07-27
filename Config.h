 //// Blynk////
#define BLYNK_PRINT Serial
#define WIFI_SSID "" //enter your WIFI name (SSID) here 
#define WIFI_PASSPHRASE "" //enter your WIFI password here 
//in the main function there is char auth[] = ״ ״ 
// make sure to insert your Blynk authorization token 
////////////////////////////

///// Adafruit IO/////
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "" //enter your Adafuit IO username here 
#define AIO_KEY         "" //enter your Adafuit IO key here 
//Pins For Color Sensor//
#define S0 D4
#define S1 D3
#define S2 D6
#define S3 D7
#define out D5
/////////////////////////

//preasure sensor pin
#define preasurePin A0

//servo pin
#define SERVOPIN D8

//number of presses
#define PRESSNUM 10

//level of force needed 
#define FORCE 2 // config here the level acourding to your sensor and experiment 
