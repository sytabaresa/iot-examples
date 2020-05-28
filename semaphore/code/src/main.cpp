#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define DEBUG

#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))

#define ACTIVE HIGH

#define RED_LED D1
#define YELLOW_LED D2
#define GREEN_LED D3

typedef enum
{
  nullState,
  runningState,
  stopState,
  stoppingState,
  runState
} State;
#ifdef DEBUG
const char *StatesPrint[] = {"null", "running", "stop", "stopping"};
#endif
typedef enum
{
  UNKNOWN,
  TIMER1,
  TIMER2,
} Event;
#ifdef DEBUG
const char *EventPrint[] = {"unknown", "timer1", "timer2"};
#endif

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

#ifdef DEBUG
  Serial.print("new switch: ");
  Serial.println(sequence);
#endif
}

void actions(State state)
{
  long *seq = sequence ? sequence1 : sequence2;
  switch (state)
  {
  case runningState:
    digitalWrite(RED_LED, ACTIVE);
    digitalWrite(YELLOW_LED, ACTIVE);
    digitalWrite(GREEN_LED, !ACTIVE);

    Blynk.virtualWrite(0, ACTIVE * 255);
    Blynk.virtualWrite(1, ACTIVE * 255);
    Blynk.virtualWrite(2, !ACTIVE * 255);

    timer.setTimeout(seq[0], sendTickEvent);
    break;
  case runState:
    digitalWrite(RED_LED, !ACTIVE);
    digitalWrite(YELLOW_LED, !ACTIVE);
    digitalWrite(GREEN_LED, ACTIVE);

    Blynk.virtualWrite(0, !ACTIVE * 255);
    Blynk.virtualWrite(1, !ACTIVE * 255);
    Blynk.virtualWrite(2, ACTIVE * 255);

    timer.setTimeout(seq[1], sendTickEvent);
    break;
  case stoppingState:
    digitalWrite(RED_LED, !ACTIVE);
    digitalWrite(YELLOW_LED, ACTIVE);
    digitalWrite(GREEN_LED, !ACTIVE);

    Blynk.virtualWrite(0, !ACTIVE * 255);
    Blynk.virtualWrite(1, ACTIVE * 255);
    Blynk.virtualWrite(2, !ACTIVE * 255);

    timer.setTimeout(seq[2], sendTickEvent);
    break;
  case stopState:
    digitalWrite(RED_LED, ACTIVE);
    digitalWrite(YELLOW_LED, !ACTIVE);
    digitalWrite(GREEN_LED, !ACTIVE);

    Blynk.virtualWrite(0, ACTIVE * 255);
    Blynk.virtualWrite(1, !ACTIVE * 255);
    Blynk.virtualWrite(2, !ACTIVE * 255);

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

  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "iot.laserud.co", 8080);
  Serial.println("Configurando Wifi...");

  sendTickEvent();
}

void loop()
{

  // machine
  if (event != UNKNOWN)
  {
#ifdef DEBUG
    Serial.print("event: ");
    Serial.println(event);
    Serial.print("currentState: ");
    Serial.println(currentState);
#endif
    State newState = machine[currentState][event];
    if (newState != nullState)
    {
      actions(newState);
#ifdef DEBUG
      Serial.print("newState: ");
      Serial.println(newState);
#endif
      currentState = newState;
      event = UNKNOWN;
    }
  }

  Blynk.run();
  timer.run();
}