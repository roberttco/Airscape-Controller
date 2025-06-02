#include <Arduino.h>
#include <OneButton.h>
#include <SimpleCLI.h>
#include <ArduinoJson.h>

#include "Fan.h"

#define FU_PIN 3
#define FD_PIN 6
#define TU_PIN 4
#define TD_PIN 5
#define DAMPER_PIN 7
#define FAN_PIN 10
#define SCMD_RX_PIN 8
#define SCMD_TX_PIN 9

Fan *fan;

SoftwareSerial *serial2;
SimpleCLI cli;
Command fu_cmd, fd_cmd, tu_cmd, td_cmd, ss_cmd;

// status updates
unsigned long update_interval = 0;
Command si_cmd;

void fu_cmd_handler(cmd *c)
{
    if (fan)
        fan->faster();
}

void fd_cmd_handler(cmd *c)
{
    if (fan)
        fan->slower();
}

void tu_cmd_handler(cmd *c)
{
    if (fan)
        fan->longer();
}

void td_cmd_handler(cmd *c)
{
    if (fan)
        fan->shorter();
}

void si_cmd_handler(cmd *c)
{
    Command cmd(c); // Create wrapper object

    // Get first (and only) Argument
    Argument arg = cmd.getArgument(0);

    // Get value of argument
    String argVal = arg.getValue();

    update_interval = (unsigned long)argVal.toInt();
}

void ss_cmd_handler(cmd *c)
{
    Command cmd(c); // Create wrapper object

    // Get first (and only) Argument
    Argument arg = cmd.getArgument(0);

    // Get value of argument
    String argVal = arg.getValue();

    update_interval = (unsigned long)argVal.toInt();
}

void sendStatus()
{
    JsonDocument statusJson;

    statusJson["speed"] = fan->speed();
    statusJson["timeRemain"] = fan->timeRemaining();
    statusJson["damper"] = fan->damperState();
    statusJson["updateInterval"] = update_interval;

    char buffer[100];
    serializeJson(statusJson, buffer, sizeof(buffer));

    serial2->println(buffer);
}

void setup()
{
    Serial.begin(115200); // Initialize serial communication at 115200 bps

    pinMode(DAMPER_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin as an output

    digitalWrite(LED_BUILTIN, LOW); // Turn off the LED

    serial2 = new SoftwareSerial(SCMD_RX_PIN, SCMD_TX_PIN);
    serial2->begin(9600);

    fan = new Fan(FAN_PIN,
                  DAMPER_PIN,
                  FU_PIN,
                  FD_PIN,
                  TU_PIN,
                  TD_PIN);

    fu_cmd = cli.addSingleArgCmd("fu", fu_cmd_handler);
    fd_cmd = cli.addSingleArgCmd("fd", fd_cmd_handler);
    tu_cmd = cli.addSingleArgCmd("tu", tu_cmd_handler);
    td_cmd = cli.addSingleArgCmd("td", td_cmd_handler);
    si_cmd = cli.addSingleArgCmd("si", si_cmd_handler);
    ss_cmd = cli.addSingleArgCmd("ss", ss_cmd_handler);

    serial2->println("online");
}

unsigned long last_status = 0;
void loop()
{
    if (fan)
        fan->loop();

    if (serial2->available())
    {
        // Read out string from the serial monitor
        String input = serial2->readStringUntil('+');

        // Serial.println(input);

        // Parse the user input into the CLI
        cli.parse(input);
    }

    if (update_interval != 0)
    {
        if (millis() - last_status > update_interval)
        {
            sendStatus();
            last_status = millis();
        }
    }
}
