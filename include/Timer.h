#include <Arduino.h>

const long max_time = __LONG_MAX__;
const long min_time = 0;

class Timer
{
private:
    // make _time a long so when it decremented it goes negative and simplifies
    // the decrement logic to check for the min time.  This just means one can't
    // set a time > 2^32-1 milliseconds.
    long _time;
    unsigned long _start;
    bool _timing;

    void (*expiredCallback)(void *);
    void *cbscope;

public:
    Timer(void (*timerExpiredCallback)(void *) = nullptr,
        void *scope = nullptr)
    {
        expiredCallback = timerExpiredCallback;
        cbscope = scope;

        _time = 0;
        _start = 0;
        _timing = false;
    }

    void set(unsigned long arg)
    {
        _timing = false;
        _time = arg;
        Serial.print("Time set to ");
        Serial.print(round(_time / 60000));
        Serial.println(" minutes.  Timer stopped.");
    }

    unsigned long get()
    {
        return (unsigned long) _time;
    }

    unsigned long get_elapsed()
    {
        return (_start != 0 ? millis() - _start : 0);
    }

    unsigned long get_remaining()
    {
        if (_timing && (_start != 0))
        {
            return _time - (millis() - _start);
        }
        else
        {
            return 0;
        }
    }

    void start()
    {
        _start = millis();
        _timing = true;
        Serial.println("Timer started.");
    }

    bool stop()
    {
        _timing = false;
        Serial.println("Timer stopped.");
    }

    bool isRunning()
    {
        return _timing;
    }

    bool loop()
    {
        if (_timing && (_start != 0))
        {
            if (millis() - _start >= _time)
            {
                _timing = false;
                expiredCallback(cbscope);
            }
        }
    }

    void increment(unsigned int arg) {
        Serial.println("Incrementing timer");

        if ((_time + arg) > max_time)
        {
            _time = max_time;
        }
        else
        {
            _time += (long)arg;
        }
    }

    void decrement(unsigned int arg) {
        Serial.println("Decrementing timer");

        if ((_time - arg) < min_time)
        {
            _time = min_time;
        }
        else
        {
            _time -= arg;
        }
    }
};