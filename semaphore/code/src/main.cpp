#include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <BlynkSimpleEsp8266.h>
#include <WiFi.h> // WiFi-library needed for Blynk
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h> // Include Blynk library

#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))

#define ACTIVE HIGH

#define RED_LED 0
#define YELLOW_LED 1
#define GREEN_LED 2

typedef enum
{
  nullState,
  runningState,
  stopState,
  stoppingState,
  runState
} State;

typedef enum
{
  UNKNOWN,
  TIMER1,
  TIMER2,
} Event;

State machine[5][3] = {};
State currentState = stopState; //initial state
Event event = UNKNOWN;

long sequence1[] = {3000, 8000, 3000, 5000};
long sequence2[] = {4000, 12000, 4000, 8000};

void sendTickEvent()
{
  event = TIMER1;
}

BlynkTimer timer;

int sequence = 0;

BLYNK_WRITE(V3)
{
  sequence = param.asInt();
}

void actions(State state)
{
  //reset pins
  digitalWrite(RED_LED, !ACTIVE);
  Blynk.virtualWrite(V0, !ACTIVE);
  digitalWrite(YELLOW_LED, !ACTIVE);
  Blynk.virtualWrite(V1, !ACTIVE);
  digitalWrite(GREEN_LED, !ACTIVE);
  Blynk.virtualWrite(V2, !ACTIVE);

  long *seq = sequence ? sequence1 : sequence2;
  switch (state)
  {
  case runningState:
    digitalWrite(RED_LED, ACTIVE);
    Blynk.virtualWrite(V0, ACTIVE);
    digitalWrite(YELLOW_LED, ACTIVE);
    Blynk.virtualWrite(V1, ACTIVE);

    timer.setTimeout(seq[0], sendTickEvent);
    break;
  case runState:
    digitalWrite(GREEN_LED, ACTIVE);
    Blynk.virtualWrite(V2, ACTIVE);

    timer.setTimeout(seq[1], sendTickEvent);
    break;
  case stoppingState:
    digitalWrite(YELLOW_LED, ACTIVE);
    Blynk.virtualWrite(V1, ACTIVE);

    timer.setTimeout(seq[2], sendTickEvent);
    break;
  case stopState:
    digitalWrite(RED_LED, ACTIVE);
    Blynk.virtualWrite(V0, ACTIVE);

    timer.setTimeout(seq[3], sendTickEvent);
    break;
  default:
    break;
  }
}

void setup()
{
  //machine State transition function
  machine[runningState][TIMER1] = runState;
  machine[runState][TIMER1] = stoppingState;
  machine[stoppingState][TIMER1] = stopState;
  machine[stopState][TIMER1] = runningState;

  // pin conf
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //blynk conf
  // You should get Auth Token in the Blynk App.
  // Go to the Project Settings (nut icon).
  char auth[] = "xhl5OxSuxNmHOgYQkFWKnLaEvSoFp4b5";

  // Your WiFi credentials.
  // Set password to "" for open networks.
  char ssid[] = "virusred inv";
  char pass[] = "vGvR47Gtt6BmsjAK";
  Blynk.begin(auth, ssid, pass, "iot.laserud.co", 8080);
}

void loop()
{

  // machine
  if (event != UNKNOWN)
  {
    State newState = machine[currentState][event];
    if (newState != nullState)
    {
      actions(newState);
      currentState = newState;
      event = UNKNOWN;
    }
  }

  Blynk.run();
  timer.run();
}