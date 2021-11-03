#include <Arduino.h>
#include <dsmr.h>

#include <LiquidCrystal_I2C.h>


// Firmware configuration
#define ESP32_UART_PORT 2   // Using UART2 on the Lolin D32
#define DATA_ENABLE   12  // GPIO12 is connected to data enable


// Define the Hardware serial port to recieve the telegrams UART for which we use the RX pin
// On the Lolin D32 UART2 uses pins 
HardwareSerial SerialHW(ESP32_UART_PORT);
P1Reader reader(&SerialHW, DATA_ENABLE);


LiquidCrystal_I2C lcd(0x27, 16, 2);   


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


/**
 * This illustrates looping over all parsed fields using the
 * ParsedData::applyEach method.
 *
 * When passed an instance of this Printer object, applyEach will loop
 * over each field and call Printer::apply, passing a reference to each
 * field in turn. This passes the actual field object, not the field
 * value, so each call to Printer::apply will have a differently typed
 * parameter.
 *
 * For this reason, Printer::apply is a template, resulting in one
 * distinct apply method for each field used. This allows looking up
 * things like Item::name, which is different for every field type,
 * without having to resort to virtual method calls (which result in
 * extra storage usage). The tradeoff is here that there is more code
 * generated (but due to compiler inlining, it's pretty much the same as
 * if you just manually printed all field names and values (with no
 * cost at all if you don't use the Printer).
 */
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


void setup()
{
  Serial.begin(115200);
  SerialHW.begin(115200);

  // configure GPIO12 --> enable the datafeed
  pinMode(DATA_ENABLE, OUTPUT);
  digitalWrite(DATA_ENABLE, 0);

  lcd.init();               // initialize the lcd 16 cols, 2 rows
  lcd.clear();
  lcd.backlight();
  

  // start a read right away
  reader.enable(false); // enable permenantly, not once !

  // Print a message on both lines of the LCD.
  /*
  lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
  lcd.print("Hello world!");
  
  lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
  lcd.print("LCD Tutorial");
  */
}


void loop()
{
/*
  while(SerialHW.available()) {
    Serial.print(char(SerialHW.read()));
  }
*/
  if (reader.loop())
  {
    MyData data;
    String err;
    if (reader.parse(&data, &err))
    {
      // Parse succesful, print result
      data.applyEach(Printer());
   
      lcd.clear();
      lcd.setCursor(0,0);
      //lcd.printf("V:%.2f, A:%.2f", data.voltage_l1.val(), data.current_l1.val() );
      lcd.printf("Power: %6.3f %s", data.power_delivered.val(), data.power_delivered::unit() );

      lcd.setCursor(0,1);
      lcd.printf("Gas  : %6.3f %s", data.gas_delivered_be.val(), data.gas_delivered_be::unit() );
    }
    else
    {
      // Parser error, print error
      Serial.println(err);
    }

  }
/*
  // clear the screen
  lcd.clear();
  delay(500);
  // read all the available characters
  //while (Serial.available() > 0) {
    // display each character to the LCD
  //  lcd.write(Serial.read());
  //}
  lcd.home();
  lcd.print("Booting...");
  delay(500);
*/


}
