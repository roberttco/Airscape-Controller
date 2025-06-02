#include <Arduino.h>

class Damper
{
private:
    enum
    {
        CLOSED,
        OPENING,
        OPEN,
        CLOSING
    } state,
      next_state, desired_state;

    unsigned long timeToOpen;
    unsigned long openStartTime;
    unsigned long timeToClose;
    unsigned long closeStartTime;

    uint8_t pin;

    bool openCallbackCalled = false;
    bool closedCallbackCalled = false;

    void (*openCallback)(void *);
    void (*closedCallback)(void *);
    void *cbscope;

    void open_damper()
    {
        Serial.println("damper.open_damper");
        digitalWrite(pin, HIGH);
    }

    void close_damper()
    {
        Serial.println("damper.close_damper");
        digitalWrite(pin, LOW);
    }

public:
    Damper(uint8_t damper_pin,
        void (*damperOpenCallback)(void *) = nullptr,
        void (*damperClosedCallback)(void *) = nullptr,
        void *scope = nullptr)
    {
        Serial.println("damper.damp er called");
        timeToOpen = 10000; // 10 seconds
        openStartTime = 0;
        timeToClose = 10000; // 10 seconds
        closeStartTime = 0;
        pin = damper_pin;

        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        state = CLOSED;
        next_state = CLOSED;
        desired_state = CLOSED;

        openCallback = damperOpenCallback;
        closedCallback = damperClosedCallback;
        cbscope = scope;
    }

    void loop()
    {
        switch (state)
        {
        case CLOSED:
            if (!closedCallbackCalled)
            {
                closedCallbackCalled = true;
                Serial.println("Damper closed");
                if (closedCallback != nullptr) closedCallback(cbscope);
                // TODO: call closed callback
            }
            switch (desired_state)
            {
            case OPEN:
                open_damper();
                openStartTime = millis();
                next_state = OPENING;
            default:
                break;
            }
            break;

        case OPENING:
            openCallbackCalled = false;
            // Serial.print("state = opening, desired_state = ");
            // Serial.print(desired_state);

            switch (desired_state)
            {
            case OPEN:
                if ((millis() - openStartTime) >= timeToOpen)
                {
                    next_state = OPEN;
                }
                break;
            case CLOSED:
                close_damper();
                closeStartTime = millis();
                next_state = CLOSING;
                break;
            default:
                break;
            }
            break;
        case OPEN:
            if (!openCallbackCalled)
            {
                openCallbackCalled = true;
                Serial.println("Damper open");
                if (openCallback != nullptr) openCallback(cbscope);
                // TODO: call open callback
            }

            switch (desired_state)
            {
            case CLOSED:
                close_damper();
                closeStartTime = millis();
                next_state = CLOSING;
                break;
            default:
                break;
            }
            break;
        case CLOSING:
            closedCallbackCalled = false;
            //Serial.println("state = closing");
            switch (desired_state)
            {
            case OPEN:
                open_damper();
                openStartTime = millis();
                next_state = OPENING;
                break;
            case CLOSED:
                if ((millis() - closeStartTime) >= timeToClose)
                {
                    next_state = CLOSED;
                }
                break;
            default:
                break;
            }
            break;
        default:
            Serial.print("Unhandled state: ");
            Serial.println(state);
            break;
        }

        state = next_state;
    }

    void open()
    {
        //Serial.println("damper.open called");
        desired_state = OPEN;
    }

    void close()
    {
        //Serial.println("damper.close called");
        desired_state = CLOSED;
    }

    bool isClosed()
    {
        //Serial.println("damper.is_closed called");
        return (state == CLOSED);
    }

    bool isOpen()
    {
        //Serial.println("damper.is_open called");
        return (state == OPEN);
    }
};