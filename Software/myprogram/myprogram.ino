#include <SD.h>
#include <SPI.h>
#include <OneWire.h> //Version 2.3.4


/* --- Configuration --- */
#define LOOP_TIME_MS        2000

/* --- Analog Pins --- */
#define ALL_BATTERIES_PIN       A5

/* --- Digital Pins --- */
const byte TEMPERATURE_SENSOR_PIN = 7;  // Temperature Sensor
const byte CSPIN                  = 53; // SD card shield

/* --- Global variables --- */
String        columnNames         = "Timestamp; Single Battery1 Voltage; Single Battery1 Voltage; Total Battery Voltage; Total Battery Level; Temperature ";
String        dataString;// holds the data to be written to the SD card 
unsigned long time;
OneWire ds(TEMPERATURE_SENSOR_PIN);

/* ----------------------- LOCAL FUNCTIONS ----------------------- */
float getTotalVoltage()
{
  float total_voltage = -1;
  total_voltage = float(analogRead(ALL_BATTERIES_PIN)) * (5.00 / 1023.00);
  return total_voltage;
}



void writeOnSDcard (String dataString)
{
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) // if the file is available, write to it
  {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);// only for debugging
  }
 
  else // if the file isn't open, pop up an error
  {
    Serial.println("error opening datalog.csv");
  }
}


float getTemperatureInCelsius() 
{
  float invalid_temperature = -300;
  byte  data[9], addr[8];
  // data[] : Data read from the scratchpad
  // addr[] : Address of 1-Wire detected sensor

  float temperature = invalid_temperature;
 
  // Look for the next 1-Wire sensor available
  if (!ds.search(addr)) 
  {
    Serial.println("No sensor found.");
    return invalid_temperature;
  }
  
  // Check the received address
  if (OneWire::crc8(addr, 7) != addr[7]) 
  {
    Serial.println("Invalid address.");
    return invalid_temperature;
  }
 
  // Make sure that the sensor type is DS18B20
  if (addr[0] != 0x28) 
  {
    Serial.println("Invalid sensor.");
    return -1;
  }
 
  // Reset 1-Wire bus and select the sensor
  ds.reset();
  ds.select(addr);
  
  // Take the measurement
  ds.write(0x44, 1);
  delay(800);
  
  // Reset 1-Wire bus and read the scratchpad
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  // Convert raw temperature into Celsius degree
  temperature = (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
  
  return temperature;
}
/*---------------------------------------------*/

void setup() 
{
  Serial.begin(9600); 

  /* --- Initialization of SD card --- */
  Serial.print("Initializing SD card...");
  pinMode(CSPIN, OUTPUT);
  
  if (!SD.begin(CSPIN))   // See if the card is present and can be initialized
  {
    Serial.println("Card failed, or not present");
    return;
  }
  
  Serial.println("Card initialized");
  writeOnSDcard(columnNames);

}

void loop() 
{
  /* ---------- Variable initialization ----------- */
  float total_voltage = -1;
  float temperature   = -500;
  time                = millis();
  dataString          = String("");


  /* ---------- Read total voltage ---------------- */
  total_voltage = getTotalVoltage();


  /* ---------- Read outdoor temperature ---------- */
  temperature = getTemperatureInCelsius();

  /* ---------- Write on SD card ------------------ */
  dataString += time;
  dataString += ';';
  dataString += total_voltage;
  dataString += ';';
  dataString += temperature;
  writeOnSDcard(dataString);


  // Print out values
  Serial.print("Total Voltage (V): ");
  Serial.println(total_voltage);
  Serial.print("Temperature (°C)");
  Serial.println(temperature);

  delay(LOOP_TIME_MS); 

}
