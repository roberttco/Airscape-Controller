#include <Arduino.h>
#include <SoftPWM.h>
#include <OneButton.h>

#include "Damper.h"

SOFTPWM_DEFINE_CHANNEL(0, DDRB, PORTB, PORTB2); // Arduino pin 10
SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(1, 100);

class Fan
{
private:
    OneButton fu_button, fd_button, tu_button, td_button;
    int speed;
    Damper *damper;

    void printSpeed()
    {
        Serial.print("Fan speed: ");
        Serial.print(speed);
        Serial.println("%");
    }

    void fanUpClicked()
    {
        speed += 10;
        if (speed > 100)
            speed = 100;

        if (damper->isClosed())
        {
            Serial.println("Damper is closed.  Speed increased.  Opening damper");
            damper->open();
        }
        else if (damper->isOpen())
        {
            Palatis::SoftPWM.set(0, speed);
        }
        printSpeed();
    }

    void fanDownClicked()
    {
        speed -= 10;
        if (speed <= 0)
        {
            speed = 0;
            Palatis::SoftPWM.set(0, speed);
            Serial.println("Damper is open.  Speed is 0.  Closing damper");
            damper->close();
        }
        else if (damper->isOpen())
        {
            Palatis::SoftPWM.set(0, speed);
        }

        printSpeed();
    }

    void timerUpClicked()
    {
    }

    void timerDownClicked()
    {
    }

    static void damperOpenCallback(void *scope)
    {
        Serial.println("damperOpen callback called");

        Fan *a = (Fan *)scope;

        Serial.println(a->speed);

        Palatis::SoftPWM.set(0, a->speed);

        digitalWrite(LED_BUILTIN,HIGH);
    }

    static void damperClosedCallback(void *arg)
    {
        Serial.println("damperClosed callback called");

        digitalWrite(LED_BUILTIN,LOW);
    }

public:
    explicit Fan(uint8_t fan_pin,
        uint8_t damper_pin,
        uint8_t fu_pin,
        uint8_t fd_pin,
        uint8_t tu_pin,
        uint8_t td_pin) : fu_button(fu_pin), fd_button(fd_pin), tu_button(tu_pin), td_button(td_pin)
    {
        Palatis::SoftPWM.begin(80);

        // print interrupt load for diagnostic purposes
        Palatis::SoftPWM.printInterruptLoad();

        speed = 0;

        pinMode(fan_pin, OUTPUT);
        damper = new Damper(damper_pin,damperOpenCallback,damperClosedCallback,this);

        fu_button.attachClick([](void *scope)           { ((Fan *)scope)->fanUpClicked(); }, this);
        fu_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->fanUpClicked(); }, this);
        fd_button.attachClick([](void *scope)           { ((Fan *)scope)->fanDownClicked(); }, this);
        fd_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->fanDownClicked(); }, this);

        tu_button.attachClick([](void *scope)           { ((Fan *)scope)->timerUpClicked(); }, this);
        tu_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->timerUpClicked(); }, this);
        td_button.attachClick([](void *scope)           { ((Fan *)scope)->timerDownClicked(); }, this);
        td_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->timerDownClicked(); }, this);

        fu_button.setLongPressIntervalMs(250);
        fd_button.setLongPressIntervalMs(250);
        tu_button.setLongPressIntervalMs(250);
        td_button.setLongPressIntervalMs(250);
    }

    // probably dont really need this since the fan is never out of scope
    ~Fan()
    {
    }

    void loop()
    {
        if (damper)
            damper->loop();

        fu_button.tick();
        fd_button.tick();
    }

    void faster()
    {
        if (damper->isClosed())
        {
            damper->open();
        }
        fanUpClicked();
    }

    void slower()
    {
        fanDownClicked();
        if (damper->isOpen())
        {
            damper->close();
        }
    }
};