#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define DEBUG

#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))

#define ACTIVE HIGH

// real pins
#define RED_LED D1
#define YELLOW_LED D2
#define GREEN_LED D3

// virtual pins
#define RED_V_LED 0
#define YELLOW_V_LED 1
#define GREEN_V_LED 2

#define SWITCH1_V_PIN 3
#define CHANGE_V_BUTTON 4
#define STATE_PRINT 5

typedef enum
{
  nullState,
  runningState,
  stopState,
  stoppingState,
  runState
} State;
#ifdef DEBUG
const char *StatesPrint[] = {"null", "running", "stop", "stopping", "run"};
#endif
typedef enum
{
  UNKNOWN,
  TIMER1,
  TIMER2,
  CHG,
} Event;
#ifdef DEBUG
const char *EventPrint[] = {"unknown", "timer1", "timer2", "change"};
#endif

State machine[5][4] = {};
State currentState = stopState; //default state
Event event = UNKNOWN;

long sequence1[] = {3000, 8000, 3000, 5000};
long sequence2[] = {4000, 12000, 4000, 8000};

void sendTickEvent()
{
  event = TIMER1;
}

BlynkTimer timer;
int timerPointer;

int sequence = 0;

// change the sequence on switch change
BLYNK_WRITE(SWITCH1_V_PIN_V_PIN)
{

  sequence = param.asInt();

#ifdef DEBUG
  Serial.print("new switch: ");
  Serial.println(sequence);
#endif
}

// make a "intantaneus" transition of the machine
BLYNK_WRITE(CHANGE_V_BUTTON)
{
  event = CHG;
}

// cycle of states
boolean redSeq[] = {!ACTIVE, ACTIVE, ACTIVE, !ACTIVE, !ACTIVE};
boolean yellowSeq[] = {!ACTIVE, ACTIVE, !ACTIVE, ACTIVE, !ACTIVE};
boolean greenSeq[] = {!ACTIVE, !ACTIVE, !ACTIVE, !ACTIVE, ACTIVE};

void actions(State state)
{
  timer.deleteTimer(timerPointer);
  long *seq = sequence ? sequence1 : sequence2;

  digitalWrite(RED_LED, redSeq[state]);
  digitalWrite(YELLOW_LED, yellowSeq[state]);
  digitalWrite(GREEN_LED, greenSeq[state]);

  Blynk.virtualWrite(RED_V_LED, redSeq[state] * 255);
  Blynk.virtualWrite(YELLOW_V_LED, yellowSeq[state] * 255);
  Blynk.virtualWrite(GREEN_V_LED, greenSeq[state] * 255);

  timerPointer = timer.setTimeout(seq[state - 1], sendTickEvent);
}

void setup()
{
  //machine State transition function
  machine[runningState][TIMER1] = runState;
  machine[runState][TIMER1] = stoppingState;
  machine[stoppingState][TIMER1] = stopState;
  machine[stopState][TIMER1] = runningState;
  machine[stopState][CHG] = runningState;
  machine[runState][CHG] = stoppingState;

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

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Configurando Wifi...");
  Serial.println(sizeof(State));
#endif
  Blynk.begin(auth, ssid, pass, "iot.laserud.co", 8080);

  // initalize machine
  currentState = stopState; //initial state
  actions(currentState);
}

void loop()
{

  // machine poll execution approach (most simple, better interrupts)
  if (event != UNKNOWN)
  {
#ifdef DEBUG
    Serial.print("event: ");
    Serial.println(EventPrint[event]);
    Serial.print("currentState: ");
    Serial.println(StatesPrint[currentState]);
#endif
    State newState = machine[currentState][event];
    if (newState != nullState)
    {
      actions(newState);
#ifdef DEBUG
      Serial.print("newState: ");
      Serial.println(StatesPrint[newState]);
      Blynk.virtualWrite(STATE_PRINT, StatesPrint[newState]);
#endif
      currentState = newState;
      event = UNKNOWN;
    }
  }

  Blynk.run();
  timer.run();
}