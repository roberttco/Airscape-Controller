#include <Arduino.h>
#include <SoftPWM.h>
#include <OneButton.h>

#include "Damper.h"

SOFTPWM_DEFINE_CHANNEL(0, DDRB, PORTB, PORTB2);  //Arduino pin 10
SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(1, 100);

class Fan
{
private:
    OneButton fu_button, fd_button;
    int speed;
    Damper *damper;

    void printSpeed()
    {
        Serial.print("Fan speed: ");
        Serial.print(speed);
        Serial.println("%");
    }

    void upClicked()
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
            Palatis::SoftPWM.set(0,speed);
        }
        printSpeed();
    }

    void downClicked()
    {
        speed -= 10;
        if (speed <= 0)
        {
            speed = 0;
            Palatis::SoftPWM.set(0,speed);
            Serial.println("Damper is open.  Speed is 0.  Closing damper");
            damper->close();
        }
        else if (damper->isOpen())
        {
            Palatis::SoftPWM.set(0,speed);
        }

        printSpeed();
    }

    void upLongPressed()
    {
        upClicked();
    }

    void downLongPressed()
    {
        downClicked();
    }

public:
    explicit Fan(uint8_t fan_pin, uint8_t damper_pin, uint8_t fu_pin, uint8_t fd_pin) : fu_button(fu_pin), fd_button(fd_pin)
    {
        Palatis::SoftPWM.begin(80);

          // print interrupt load for diagnostic purposes
        Palatis::SoftPWM.printInterruptLoad();

        speed = 0;

        pinMode(fan_pin, OUTPUT);

        damper = new Damper(damper_pin);

        fu_button.attachClick([](void *scope)
                              { ((Fan *)scope)->upClicked(); }, this);
        fu_button.attachDuringLongPress([](void *scope)
                                       { ((Fan *)scope)->upLongPressed(); }, this);
        fd_button.attachClick([](void *scope)
                              { ((Fan *)scope)->downClicked(); }, this);
        fd_button.attachDuringLongPress([](void *scope)
                                       { ((Fan *)scope)->downLongPressed(); }, this);

        fu_button.setLongPressIntervalMs(250);
        fd_button.setLongPressIntervalMs(250);
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
        upClicked();
    }

    void slower()
    {
        downClicked();
        if (damper->isOpen())
        {
            damper->close();
        }
    }
};