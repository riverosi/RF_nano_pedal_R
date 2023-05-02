/*  RF Nano Example
This code is also compatible with an RF Nano (Arduino Nano with integrated NRF24L01+ module)
<---- Pins for Radio:
CE - Arduino pin 7
CSN - Arduino pin 8
SCK - Arduino pin 13
MOSI - Arduino pin 11
MISO - Arduino pin 12
<---- Pins for hx711
DT - Arduino pin D2
SCH - Arduino pin D3
*/
#include <Arduino.h>
#include <RF24.h>
#include <printf.h>
#include <HX711_ADC.h>
#include <avr/wdt.h>
/**
 * Uncommet to run the selected program
*/
//#define TEST 1
#define APP 1
/**
 * SELECT PEDAL TO DEBUG
*/
#define PEDAL_R 1
//#define PEDAL_L 1

/*Pins for the nrf24l01+ in the Arduino Nano RF24*/
#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);//Radio object whith pins definitions

uint8_t datos[32];//buffer data for send to PRX

byte addr_pedal_R[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Address for the pedal R
byte addr_pedal_L[5] = { 0xC2, 0xC2, 0xC2, 0xC2, 0xC2 }; // Address for the pedal L

/*Pins for HX711 adc*/
const int HX711_dout = 2;
const int HX711_sck = 3;
HX711_ADC CELDA_CARGA(HX711_dout, HX711_sck);//HX711_ADC object whith pins definitions

/*Function: converts input float variable to byte array*/
void float2Bytes(float val, byte *bytes_array);

void setup()
{
  wdt_disable(); // Disable wdt
  //RADIO SETUP
  radio.begin();
  Serial.begin(9600);
  printf_begin();
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.setAddressWidth(5);
  radio.setRetries(0, 3);
  radio.setCRCLength(RF24_CRC_8);
  radio.setChannel(0x02);
  radio.setDataRate(RF24_2MBPS);
  #ifdef PEDAL_R //Select pedal compilation
    radio.openReadingPipe(0, addr_pedal_R);
    radio.openWritingPipe(addr_pedal_R);
  #else
    radio.openReadingPipe(0, addr_pedal_L);
    radio.openWritingPipe(addr_pedal_L);
  #endif
  radio.stopListening();
  radio.printDetails();
  //HX711ADC SETUP
  CELDA_CARGA.begin();
  float Valor_Calibracion = 625531.25f; // VALOR ENCONTRADO TRAS CORRER PROGRAMA DE CALIBRACIÓN
  long T_estabiliza = 200;             //MEJORA PRECISION EN ARRANQUE
  boolean dotare = true;                 //NECESARIO PARA TARAR SIEMPRE AL COMIENZO
  delay(1000); /*Delay from start tare*/
  CELDA_CARGA.start(T_estabiliza, dotare);
  if (CELDA_CARGA.getTareTimeoutFlag())
  {
    Serial.println("Failure starting loadcell");
    while (1);
  }
  else
  {
    CELDA_CARGA.setCalFactor(Valor_Calibracion);
    Serial.println("Successfull start");
    delay(1000);
    Serial.println("The values of force are measured in [Kg]");
    delay(1000);
  }

  wdt_enable(WDTO_250MS); //Config watchdog every 250 ms
}

void loop()
{
  wdt_reset(); // Reset watchdog
  // put your main code here, to run repeatedly:
  if (CELDA_CARGA.update())
  {
    #ifdef APP /*APP Program*/
    #ifdef PEDAL_R
    datos[0] = (uint8_t)0x02;
    float i = CELDA_CARGA.getData();
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_R);
    radio.write(datos, 32);
    Serial.print(datos[0]);
    Serial.print('\t');
    Serial.println(i);
    #else
    datos[0] = (uint8_t)0x01;
    float i = CELDA_CARGA.getData();
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_L);
    radio.write(datos, 32);
    Serial.print(datos[0]);
    Serial.print('\t');
    Serial.println(i);
    #endif
    #endif
    
    #ifdef TEST /*TEST Program*/
    #ifdef PEDAL_R
    datos[0] = (uint8_t)0x02;
    float i = 5.0f;
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_R);
    radio.write(datos, 32);
    delay(100);
    i = 0.0f;
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_R);
    radio.write(datos, 32);
    delay(100);
    #else
    datos[0] = (uint8_t)0x01;
    float i = 5.0f;
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_L);
    radio.write(datos, 32);
    delay(500);
    i = 0.0f;
    float2Bytes(i, &datos[1]);
    radio.openWritingPipe(addr_pedal_L);
    radio.write(datos, 32);
    delay(500);
    #endif
    #endif
  }
}

//Function: converts input float variable to byte array
void float2Bytes(float val, byte *bytes_array)
{
  // Create union of shared memory space
  union
  {
    float float_variable;
    byte temp_array[4];
  } u;
  // Overite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}
