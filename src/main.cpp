#include <Arduino.h>
#include "dsmr.h"

HardwareSerial SerialHW(2);
P1Reader reader(&SerialHW, 0);


/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
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
    /* TimestampedFixedValue */ gas_delivered,
    /* uint16_t */ thermal_device_type,
    /* String */ thermal_equipment_id,
    /* uint8_t */ thermal_valve_position,
    /* TimestampedFixedValue */ thermal_delivered,
    /* uint16_t */ water_device_type,
    /* String */ water_equipment_id,
    /* uint8_t */ water_valve_position,
    /* TimestampedFixedValue */ water_delivered,
    /* uint16_t */ sub_device_type,
    /* String */ sub_equipment_id,
    /* uint8_t */ sub_valve_position,
    /* TimestampedFixedValue */ sub_delivered>;


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

#define ENABLE        12  // GPIO12 is connected to data enable
#define SWITCHTIME 10000  // switch on/off every 10 secs

unsigned long prev_time;
uint8_t status = 0;

void setup()
{
  Serial.begin(115200);
  SerialHW.begin(115200);

  // configure GPIO12
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, 0);

  prev_time = millis();
}


void loop()
{
  while(SerialHW.available()) {
    Serial.print(char(SerialHW.read()));
  }

  if ( ( ( millis() - prev_time ) > SWITCHTIME ) && ( ! SerialHW.available() ) )
  {
    Serial.println("SWITCHING THE DATAFEED...");
    status = (status + 1) % 2;
    digitalWrite(ENABLE, status);

    prev_time = millis();
  }
  

}
