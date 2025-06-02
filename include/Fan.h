#include <Arduino.h>
#include <SoftPWM.h>
#include <OneButton.h>
#include <SoftwareSerial.h>

#include "Damper.h"
#include "Timer.h"

// configure the SoftPWM output
SOFTPWM_DEFINE_CHANNEL(0, DDRB, PORTB, PORTB2); // Arduino pin 10
SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(1, 100);



class Fan
{
private:
    OneButton fu_button, fd_button;
    OneButton tu_button, td_button;
    int _speed;
    Damper *damper;
    Timer *timer;
    unsigned long lasttimer = 0;

    void printSpeed()
    {
        Serial.print("Fan speed: ");
        Serial.print(_speed);
        Serial.println("%");
    }

    void fanUpClicked()
    {
        _speed += 10;
        if (_speed > 100)
            _speed = 100;

        if (damper->isClosed())
        {
            Serial.println("Damper is closed.  Speed increased.  Opening damper");
            damper->open();
        }
        else if (damper->isOpen())
        {
            Palatis::SoftPWM.set(0, _speed);
        }
        printSpeed();
    }

    void fanDownClicked()
    {
        _speed -= 10;
        if (_speed <= 0)
        {
            _speed = 0;
            Palatis::SoftPWM.set(0, _speed);
            Serial.println("Damper is open.  Speed is 0.  Closing damper");
            damper->close();
        }
        else if (damper->isOpen())
        {
            Palatis::SoftPWM.set(0, _speed);
        }

        printSpeed();
    }

    //void damperOpenCallback(void *scope)
    void damperOpenCallback()
    {
        Serial.print("Fan::damperOpen callback called. Speed set to ");
        Serial.println(_speed);
        Palatis::SoftPWM.set(0, _speed);

        digitalWrite(LED_BUILTIN,HIGH);
    }

    //void damperClosedCallback(void *arg)
    void damperClosedCallback()
    {
        Serial.println("Fan::damperClosed callback called");

        digitalWrite(LED_BUILTIN,LOW);
    }


    void timerUpClicked()
    {
        Serial.print ("Fan::timerUpClicked called. Time now set to ");
        timer->increment(10000);
        Serial.println(timer->get());

        if (_speed == 0)
        {
            // this will open/reopen the damper and set the speed to 10%
            fanUpClicked();
        }

        timer->start();
    }

    void timerDownClicked()
    {
        Serial.print ("Fan::timerDownClicked called. Time now set to ");
        timer->decrement(10000);
        Serial.println(timer->get());
    }

    void timerExpiredCallback()
    {
        Serial.println("Fan::timerExpiredCallback called");

        _speed = 0;
        Palatis::SoftPWM.set(0, _speed);
        damper->close();
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

        _speed = 0;

        pinMode(fan_pin, OUTPUT);
        damper = new Damper(damper_pin,
            [](void *scope) { ((Fan *)scope)->damperOpenCallback(); },
            [](void *scope) { ((Fan *)scope)->damperClosedCallback(); },
            this);

        timer = new Timer(
            [](void *scope) { ((Fan *)scope)->timerExpiredCallback(); },
            this);

        fu_button.attachClick([](void *scope)           { ((Fan *)scope)->fanUpClicked(); }, this);
        fu_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->fanUpClicked(); }, this);
        fu_button.setLongPressIntervalMs(250);

        fd_button.attachClick([](void *scope)           { ((Fan *)scope)->fanDownClicked(); }, this);
        fd_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->fanDownClicked(); }, this);
        fd_button.setLongPressIntervalMs(250);

        tu_button.attachClick([](void *scope)           { ((Fan *)scope)->timerUpClicked(); }, this);
        tu_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->timerUpClicked(); }, this);
        tu_button.setLongPressIntervalMs(250);

        td_button.attachClick([](void *scope)           { ((Fan *)scope)->timerDownClicked(); }, this);
        td_button.attachDuringLongPress([](void *scope) { ((Fan *)scope)->timerDownClicked(); }, this);
        td_button.setLongPressIntervalMs(250);
    }

    // probably dont really need this since the fan is never out of scope
    ~Fan()
    {
    }

    void faster()
    {
        this->fanUpClicked();
    }

    void slower()
    {
        this->fanDownClicked();
    }

    void setSpeed(int percent)
    {
        _speed = max(0, std::min(percent, 100));
        Palatis::SoftPWM.set(0, _speed);
    }

    void longer()
    {
        this->timerUpClicked();
    }

    void shorter()
    {
        this->timerDownClicked();
    }

    int speed()
    {
        return this->_speed;
    }

    bool damperState()
    {
        return this->damper->isOpen();
    }

    unsigned long timeRemaining()
    {
        return this->timer->get_remaining();
    }

    void loop()
    {
        if (damper)
            damper->loop();

        if (timer && timer->isRunning())
        {
            timer->loop();
            if ((millis() - lasttimer) > 1000)
            {
                lasttimer = millis();
                Serial.println(timer->get_remaining());
            }
        }

        fu_button.tick();
        fd_button.tick();
        tu_button.tick();
        td_button.tick();
    }
};