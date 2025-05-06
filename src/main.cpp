#include <Arduino.h>
#include "OneButton.h"

#include "Fan.h"

#define FU_PIN 3
#define FD_PIN 4
#define TU_PIN 5
#define TD_PIN 6
#define DAMPER_PIN 7
#define FAN_PIN 10

Fan *fan;

int damper_state = 0; // Damper state (0-100)

void setup()
{
    Serial.begin(115200); // Initialize serial communication at 9600 bps

    pinMode(DAMPER_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);   // Initialize the built-in LED pin as an output
    digitalWrite(LED_BUILTIN, LOW); // Turn off the LED

    fan = new Fan(FAN_PIN,DAMPER_PIN,FU_PIN,FD_PIN);
}

void loop()
{
    if (fan)
        fan->loop();
}

