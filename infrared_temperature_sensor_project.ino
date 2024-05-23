#define ANALOG_READ_PIN 2


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  auto analog_value = analogRead(ANALOG_READ_PIN);
  double voltage = (analog_value / 4095) * 3.3;
  Serial.println(voltage);
}
