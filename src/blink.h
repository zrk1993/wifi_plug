#ifndef BLink_h
#define BLink_h

#include <Arduino.h>

void blink(int times, int dl)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    bool led_on = false;
    for (int i = 0; i <= times * 2; i++)
    {
        delay(dl);
        led_on = !led_on;
        digitalWrite(LED_BUILTIN, led_on ? LOW : HIGH);
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void blink_on () {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void blink_off () {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void blink(int times)
{
    blink(times, 100);
}

void blink()
{
    blink(2, 100);
}

void blink_err()
{
    blink(3, 150);
}

void blink_ok()
{
    blink(3, 300);
}

#endif