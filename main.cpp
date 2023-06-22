#include <Arduino.h>

#define SEAT 6   // Lid up switch is connected to digital 6
#define BUZZER 8 // Buzzer is connected to digital 8
#define FSR A0   // FSR is connected to analog 0
#define SWITCH 7 // Digital switch to determine whether to run the code

#define IS_UP 0             // When all is up, no pressure dected
#define IS_SEAT_DOWN 20      // Pressure when just the seat is down
#define IS_SITTING 500      // Pressure when someone is sitting on the seat
#define TIMEOUT 200         // Time (in millis) before buzzer goes off
#define HOUR_MILLIS 3600000 // One hour in millis

int fsrReading;                       // the analog reading from the FSR resistor divider
int timer = 0;                        // When someone stands up, give them a grace period before buzzing
bool shouldBuzz = false;              // Whether the timer has expired
bool isConnected = true;              // If this is false, don't buzz
bool isUp = false;                    // If the seat lid is up

enum STATE
{
    SEAT_DOWN, // 0: Seat down, lid up
    LID_DOWN,  // 1: Lid down, seat down
    SITTING,   // 2: Lid up, seat down, sitting on seat
    UP         // 3: Lid up, seat up
};

STATE currentState = LID_DOWN;

void setup(void)
{
    Serial.begin(9600);
    pinMode(BUZZER, OUTPUT);
    pinMode(SWITCH, INPUT_PULLUP);
    pinMode(SEAT, INPUT_PULLUP);
}

void loop(void)
{
    fsrReading = analogRead(FSR);

    isConnected = !digitalRead(SWITCH);
    isUp = !digitalRead(SEAT);

    if (isConnected)
    {
        return;
    }

    Serial.print("State: ");
    Serial.print(currentState);
    Serial.print(" Timer: ");
    Serial.print(timer);
    Serial.print(" Reading: ");
    Serial.print(fsrReading);
    Serial.print(" Is up? ");
    Serial.println(isUp);

    if (timer > 0)
    {
        timer--;
    }
    else
    {
        if (currentState == SEAT_DOWN || currentState == UP)
        {
            shouldBuzz = true;
        }
    }

    if (shouldBuzz && timer == 0 && isUp)
    {
        tone(BUZZER, 4096, 500);
    }
    else
    {
        noTone(BUZZER);
    }

    switch (currentState)
    {
    case SEAT_DOWN:
        if (!isUp)
        {
            currentState = LID_DOWN;
            shouldBuzz = false;
        }
        else if (fsrReading >= IS_SITTING)
        {
            currentState = SITTING;
            shouldBuzz = false;
        }
        else if (isUp && fsrReading < IS_SEAT_DOWN)
        {
            currentState = UP;
        }
        break;
    case LID_DOWN:
        if (fsrReading >= IS_SEAT_DOWN && isUp)
        {
            currentState = SEAT_DOWN;
            if (timer == 0 && !shouldBuzz)
            {
                timer = TIMEOUT;
            }
        }
        else if (fsrReading >= IS_UP && fsrReading < IS_SEAT_DOWN)
        {
            currentState = UP;
        }
        break;
    case UP:
        if (!isUp && fsrReading < IS_SITTING)
        {
            currentState = LID_DOWN;
        }
        else if (fsrReading >= IS_SEAT_DOWN && isUp)
        {
            currentState = SEAT_DOWN;
            if (timer == 0 && !shouldBuzz)
            {
                timer = TIMEOUT;
            }
        }
        break;
    case SITTING:
        if (!isUp && fsrReading < IS_SITTING)
        {
            currentState = LID_DOWN;
        }
        else if (fsrReading >= IS_SEAT_DOWN && fsrReading < IS_SITTING && isUp)
        {
            currentState = SEAT_DOWN;
            if (timer == 0 && !shouldBuzz)
            {
                timer = TIMEOUT;
            }
        }
        else if (fsrReading >= IS_UP && fsrReading < IS_SEAT_DOWN)
        {
            currentState = UP;
        }
    default:
        break;
    }
}