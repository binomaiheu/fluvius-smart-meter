#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <IPAddress.h>

#undef USE_LCD
#ifdef USE_LCD
# include <LiquidCrystal_I2C.h>
#endif

#include <dsmr.h>
#include "arduino_secrets.h"


// =================================================================
// Defines
// =================================================================
#define ESP32_UART_PORT   2  // Using UART2 on the Lolin D32
#define DATA_ENABLE      12  // GPIO12 is connected to data enable

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )



// =================================================================
// Globals
// =================================================================
WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient  ntpClient(ntpUDP);
HardwareSerial SerialHW(ESP32_UART_PORT);
P1Reader reader(&SerialHW, DATA_ENABLE);

void callback(char *topic, byte* payload, unsigned int len);
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, callback, wifiClient);

unsigned long tWiFiConnect;
unsigned long tMQTTConnect;

#ifdef USE_LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);   
#endif

using MyData = ParsedData<
    /* String */ identification,
    /* String */ p1_version,
    /* String */ timestamp,
    /* String */ equipment_id,
    /* FixedValue */ energy_delivered_tariff1,
    /* FixedValue */ energy_delivered_tariff2,
    /* FixedValue */ energy_returned_tariff1,
    /* FixedValue */ energy_returned_tariff2,
    /* String */ electricity_tariff,
    /* FixedValue */ power_delivered,
    /* FixedValue */ power_returned,
    /* FixedValue */ electricity_threshold,
    /* uint8_t */ electricity_switch_position,
    /* uint32_t */ electricity_failures,
    /* uint32_t */ electricity_long_failures,
    /* String */ electricity_failure_log,
    /* uint32_t */ electricity_sags_l1,
    /* uint32_t */ electricity_sags_l2,
    /* uint32_t */ electricity_sags_l3,
    /* uint32_t */ electricity_swells_l1,
    /* uint32_t */ electricity_swells_l2,
    /* uint32_t */ electricity_swells_l3,
    /* String */ message_short,
    /* String */ message_long,
    /* FixedValue */ voltage_l1,
    /* FixedValue */ voltage_l2,
    /* FixedValue */ voltage_l3,
    /* FixedValue */ current_l1,
    /* FixedValue */ current_l2,
    /* FixedValue */ current_l3,
    /* FixedValue */ power_delivered_l1,
    /* FixedValue */ power_delivered_l2,
    /* FixedValue */ power_delivered_l3,
    /* FixedValue */ power_returned_l1,
    /* FixedValue */ power_returned_l2,
    /* FixedValue */ power_returned_l3,
    /* uint16_t */ gas_device_type,
    /* String */ gas_equipment_id,
    /* uint8_t */ gas_valve_position,
    /* TimestampedFixedValue */ gas_delivered_be>; // need the belgian variety..

struct Printer
{
  template <typename Item>
  void apply(Item &i)
  {
    if (i.present())
    {
      Serial.print(Item::name);
      Serial.print(F(": "));
      Serial.print(i.val());
      Serial.print(Item::unit());
      Serial.println();
    }
  }
};

void printWiFi(void)
{

  IPAddress ip = WiFi.localIP();
  long rssi = WiFi.RSSI();

  Serial.print(F("SSID: ")); 
  Serial.print(WiFi.SSID());
  Serial.print(F(", IP: "));
  Serial.print(ip);
  Serial.print(F(", RSSI: "));
  Serial.print(rssi);
  Serial.println(" dBm"); 
  
  return;
}

void startWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED ) 
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  }
  return;
}

void callback(char *topic, byte* payload, unsigned int len )
{
  return;
}


String to_time_str(unsigned long secs) {
  unsigned long rawTime = secs;
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}


// Based on https://github.com/PaulStoffregen/Time/blob/master/Time.cpp
// currently assumes UTC timezone, instead of using this->_timeOffset
String to_isoformat(unsigned long secs) {
  unsigned long rawTime = secs / 86400L;  // in days
  unsigned long days = 0, year = 1970;
  uint8_t month;
  static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
    year++;
  rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
  days=0;
  for (month=0; month<12; month++) {
    uint8_t monthLength;
    if (month==1) { // february
      monthLength = LEAP_YEAR(year) ? 29 : 28;
    } else {
      monthLength = monthDays[month];
    }
    if (rawTime < monthLength) break;
    rawTime -= monthLength;
  }
  String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
  String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
  return String(year) + "-" + monthStr + "-" + dayStr + "T" + to_time_str(secs ? secs : 0) + "Z";
}



// =================================================================
// Setup
// =================================================================
void setup()
{
  Serial.begin(115200);
  SerialHW.begin(115200);

  // configure GPIO12 --> enable the datafeed
  pinMode(DATA_ENABLE, OUTPUT);
  digitalWrite(DATA_ENABLE, 0);

#ifdef USE_LCD
  lcd.init();               // initialize the lcd 16 cols, 2 rows
  lcd.clear();
  lcd.backlight();
#endif

  // start a read right away
  reader.enable(false);

  // starting wifi, need connection before continuing
  Serial.println(F("Starting WiFI"));
  startWiFi();
  tWiFiConnect = millis();
  while ( ( WiFi.status() != WL_CONNECTED ) && ( millis() - tWiFiConnect ) < 30000 ) 
  {
    delay(100);
  }
  if ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Cannot connect to WiFi, giving up... ");
    while(true)
    {}
  }
  printWiFi();

  // starting mqtt
  mqttClient.connect("P1reader");
  tMQTTConnect = millis();
  while ( ! mqttClient.connected() && ( millis() - tMQTTConnect ) < 30000 )
  {
    delay(100);
  }
  if ( ! mqttClient.connected() ) {
    Serial.println("Unable to connect to MQTT broker, giving up...");
    while(true)
    {}
  }
  Serial.println("Connected to MQTT broker...");

  return;
}


// =================================================================
// Loop
// =================================================================
void loop()
{

  /* ========================================================================
  *  WiFi management   
  * 
  *  if the wifi is not connected and the last connection attempt is more than 
  *  30 seconds ago... attempt to start the wifi agaion
  *  ==================================================================== */
  if ( ( WiFi.status() != WL_CONNECTED ) && ( millis() - tWiFiConnect ) > 30000 ) {
    Serial.println(F("WiFi not connected, connecting..."));
    startWiFi();
    tWiFiConnect = millis();
    delay(10);
  
    // ensure we get a valid NTP
    Serial.print(F("Updating time client..."));
    ntpClient.begin();
    while ( ! ntpClient.update() ) {
      Serial.print(".");
      ntpClient.forceUpdate();
    }
    Serial.println(F("OK"));
    Serial.println("Time recieved : " + ntpClient.getFormattedTime() );

    printWiFi();
  }

  /* ========================================================================
  *  MQTT Management
  *  ==================================================================== */
  if ( ( WiFi.status() == WL_CONNECTED ) &&
      ( ! mqttClient.connected() ) && 
      ( millis() - tMQTTConnect ) > 30000 ) {    
    if ( mqttClient.connect( MQTT_CLIENT, MQTT_USER, MQTT_PASSWD ) ) {
      Serial.println("Connected to MQTT server.");
    } else {
      Serial.print("Failed to connect with MQTT server, state ");
      Serial.println(mqttClient.state());
    }
    tMQTTConnect = millis();
  }


  if (reader.loop())
  {
    MyData data;
    String err;
    if (reader.parse(&data, &err))
    {
      // Parse succesful, print result
      data.applyEach(Printer());
   
#ifdef USE_LCD      
      lcd.clear();
      lcd.setCursor(0,0);
      //lcd.printf("V:%.2f, A:%.2f", data.voltage_l1.val(), data.current_l1.val() );
      lcd.printf("Power: %6.3f %s", data.power_delivered.val(), data.power_delivered::unit() );

      lcd.setCursor(0,1);
      lcd.printf("Gas  : %6.3f %s", data.gas_delivered_be.val(), data.gas_delivered_be::unit() );
#endif

      if ( mqttClient.connected() ) 
      {
        ntpClient.update();

        ntpClient.getFormattedTime();

        String payload = 
          "{\"dateObserved\": \"" + to_isoformat(ntpClient.getEpochTime()) + "\"" +
          ",\"power\":{\"value\":" + String(data.power_delivered.val(),3) + ",\"unit\":\"" + data.power_delivered::unit() + "\"}"
          ",\"voltage\":{\"value\":" + String(data.voltage_l1.val(),3) + ",\"unit\":\"" + data.voltage_l1::unit() + "\"}"
          ",\"current\":{\"value\":" + String(data.current_l1.val(),3) + ",\"unit\":\"" + data.current_l1::unit() + "\"}"        
          ",\"gas\":{\"value\":" + String(data.gas_delivered_be.val(),3) + ",\"unit\":\"" + data.gas_delivered_be::unit() + "\"}"
        + "}";
        // Serial.println(payload);

        String chan = MQTT_CHANNEL + String("/") + MQTT_CLIENT + String("/value");
              
        if ( ! mqttClient.publish( chan.c_str(), payload.c_str()) ) {
          Serial.println(F("-- MQTT publish failed..."));
        }
      }






    }
    else
    {
      // Parser error, print error
      Serial.println(err);
    }

  }

  mqttClient.loop();
}
