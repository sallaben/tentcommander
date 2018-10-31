#include <Adafruit_Si7021.h>
#include <math.h>

const bool DEBUG              = false;

const int FAN_OUT             = 12;
const int LED_GREEN           = 11;
const int LED_RED             = 10;

      int FAN_SPEED           = 100;
const int FAN_SCALE           = 50;
const int FAN_OFF             = 0;
const int FAN_MIN             = 100;
const int FAN_MAX             = 250;

      double TEMPERATURE      = 0;
const double TEMPERATURE_MIN  = 25;
const double TEMPERATURE_MAX  = 30;

      double HUMIDITY         = 0;
const double HUMIDITY_MIN     = 50;
const double HUMIDITY_MAX     = 75;

      double VPD              = 0;
const double VPD_MIN          = 0.4;
const double VPD_MAX          = 1.4;

Adafruit_Si7021 sensor        = Adafruit_Si7021();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(FAN_OUT, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  analogWrite(FAN_OUT, 0);
  
  Serial.begin(115200);

  if (DEBUG) {
    while (!Serial) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      delay(500);
    }
  }
  
  while (!sensor.begin()) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_GREEN, HIGH);
    delay(50);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    delay(50);
  }

  if (DEBUG) {
    Serial.print("Using Adafruit Si7021 sensor");
    Serial.print(" Rev(");
    Serial.print(sensor.getRevision());
    Serial.print(")");
    Serial.print(" Serial #"); Serial.print(sensor.sernum_a, HEX); Serial.println(sensor.sernum_b, HEX);
  }
}

void loop() {
  TEMPERATURE = sensor.readTemperature();
  HUMIDITY = sensor.readHumidity();
  VPD = vpd();

  if (DEBUG) {
    Serial.print("\n\nTemperature: ");
    Serial.print(TEMPERATURE);
    Serial.print("ºC");
    Serial.print("\nHumidity: ");
    Serial.print(HUMIDITY);
    Serial.print("%");
    Serial.print("\nVPD: ");
    Serial.print(VPD);
    Serial.print("\nFan: ");
    Serial.print(FAN_SPEED);
  }
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  
  if (normal()) {
    happy();
  } else {
    sad();
  }
}

void happy() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  
  decrease_speed();
  analogWrite(FAN_OUT, FAN_SPEED);

  int total = 0;
  while (total <= 60000) {
    digitalWrite(LED_GREEN, HIGH);
    delay(4000);
    digitalWrite(LED_GREEN, LOW);
    delay(1000);
    total += 5000;
  }
}

void sad() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  
  if (!vpd_normal()) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if (!h_normal()) {
    digitalWrite(LED_GREEN, HIGH);
  }
  if (!t_normal()) {
    digitalWrite(LED_RED, HIGH);
  }
  if (t_above()) {
    increase_speed();
    if (h_below()) {
      FAN_SPEED = FAN_MAX;
    }
  } else if (t_below()) {
    decrease_speed();
    if (h_below()) {
      FAN_SPEED = FAN_OFF;
    }
  } else {
    if (h_above()) {
      increase_speed();
    } else if (h_below()) {
      FAN_SPEED = FAN_MIN;
    }
  }
  
  analogWrite(FAN_OUT, FAN_SPEED);
  delay(30000);
}

int increase_speed() {
  FAN_SPEED = min(FAN_MAX, FAN_SPEED + FAN_SCALE);
}

int decrease_speed() {
  FAN_SPEED = max(FAN_MIN, FAN_SPEED - FAN_SCALE);
}

bool normal() {
  return ((t_normal() && h_normal()) && vpd_normal());
}

bool t_normal() {
  if (t_above() || t_below()) {
    return false;
  }
  return true;
}

bool t_above() {
  if (TEMPERATURE > TEMPERATURE_MAX) {
    return true;
  }
  return false;
}

bool t_below() {
  if (TEMPERATURE < TEMPERATURE_MIN) {
    return true;
  }
  return false;  
}

bool h_normal() {
  if (h_above() || h_below()) {
    return false;
  }
  return true;
}

bool h_above() {
  if (HUMIDITY > HUMIDITY_MAX) {
    return true;
  }
  return false;
}

bool h_below() {
  if (HUMIDITY < HUMIDITY_MIN) {
    return true;
  }
  return false;
}

  // Temperature (Celsius)
  // Relative Humidity (%)
double vpd() {
  double TEMP_LEAF = TEMPERATURE - 1.5;
  double VPconst = 0.6107;
  double VPsat = VPconst * pow(10, ((7.5 * TEMP_LEAF) / (273.15 + TEMP_LEAF)));
  double VPair = VPconst * (HUMIDITY / 100) * pow(10, ((7.5 * TEMPERATURE) / (273.15 + TEMPERATURE)));
  return (VPsat - VPair);
}

bool vpd_normal() {
  if (vpd_above() || vpd_below()) {
    return false;
  }
  return true;
}

bool vpd_above() {
  if (VPD > VPD_MAX) {
    return true;
  }
  return false;
}

bool vpd_below() {
  if (VPD < VPD_MIN) {
    return true;
  }
  return false;  
}
