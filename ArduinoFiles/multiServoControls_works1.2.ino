/*  // Debugged  Works 

Have to set it back to initial position, $00000, after turning on a servo

1 = on
0 = off

  5-Servo Hand Controller (Serial command: $00000)

  Format:
    $<d0><d1><d2><d3><d4>

  By default in THIS SCRIPT, the digits map like:
    d0 = THUMB
    d1 = INDEX
    d2 = MIDDLE
    d3 = RING
    d4 = PINKY

  Each digit is '0' or '1':
    0 -> servo goes to 0 degrees
    1 -> servo goes to 180 degrees

  Serial Monitor tips:
    - Baud: 9600
    - Line ending: "No line ending" (recommended)
      (Script ignores \r and \n anyway.)
*/

#include <Servo.h>

// -------------------------
// CONFIG
// -------------------------
#define NUM_SERVOS         5
#define DIGITS_PER_VALUE   1                  // 1 digit per servo (0/1)
const int STRING_LENGTH = (NUM_SERVOS * DIGITS_PER_VALUE) + 1; // +1 for '$'

// Servo pins (change as needed)
const int PIN_THUMB  = 7;
const int PIN_INDEX  = 8;
const int PIN_MIDDLE = 9;
const int PIN_RING   = 10;
const int PIN_PINKY  = 11;

// -------------------------
// GLOBALS
// -------------------------
Servo servoThumb, servoIndex, servoMiddle, servoRing, servoPinky;

int valsRec[NUM_SERVOS] = {0, 0, 0, 0, 0};

int counter = 0;
bool counterStart = false;
String receivedString = "";

// -------------------------
// FUNCTIONS
// -------------------------
void applyToServos()
{
  // d0..d4 => Thumb..Pinky (as described at top)
  servoThumb.write (valsRec[0] ? 360 : 0);
  servoIndex.write (valsRec[1] ? 360 : 0);
  servoMiddle.write(valsRec[2] ? 360 : 0);
  servoRing.write  (valsRec[3] ? 360 : 0);
  servoPinky.write (valsRec[4] ? 360 : 0);
}

bool isValidDigit(char c)
{
  return (c >= '0' && c <= '9'); // allows >1 too, but we'll treat any non-zero as "on"
}

void receiveData()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();

    // Ignore newline / carriage return (Serial Monitor line ending settings)
    if (c == '\n' || c == '\r') continue;

    // Start of packet
    if (c == '$')
    {
      counterStart = true;       // FIXED: assignment (not ==)
      receivedString = "";
      counter = 0;
    }

    if (!counterStart) continue;

    // Collect exactly STRING_LENGTH chars: "$" + 5 digits
    if (counter < STRING_LENGTH)
    {
      receivedString += c;       // safer than String(receivedString + c)
      counter++;
    }

    // Once we have full packet, parse it
    if (counter >= STRING_LENGTH)
    {
      // Basic validation: must start with '$' and have enough digits
      if (receivedString.length() == STRING_LENGTH && receivedString[0] == '$')
      {
        for (int i = 0; i < NUM_SERVOS; i++)
        {
          int idx = 1 + (i * DIGITS_PER_VALUE); // skip '$'
          char d = receivedString[idx];

          // If malformed digit, treat as 0
          if (!isValidDigit(d)) {
            valsRec[i] = 0;
          } else {
            // "0" -> 0, any other digit -> 1 (so '1'..'9' become ON)
            valsRec[i] = (d == '0') ? 0 : 1;
          }
        }

        // Optional debug print
        Serial.print("RX: ");
        Serial.print(receivedString);
        Serial.print("  =>  [T I M R P] = ");
        for (int i = 0; i < NUM_SERVOS; i++) {
          Serial.print(valsRec[i]);
          if (i < NUM_SERVOS - 1) Serial.print(" ");
        }
        Serial.println();

        applyToServos();
      }

      // Reset for next packet
      receivedString = "";
      counter = 0;
      counterStart = false;
    }
  }
}

// -------------------------
// SETUP / LOOP
// -------------------------
void setup()
{
  Serial.begin(9600);

  servoThumb.attach(PIN_THUMB);
  servoIndex.attach(PIN_INDEX);
  servoMiddle.attach(PIN_MIDDLE);
  servoRing.attach(PIN_RING);
  servoPinky.attach(PIN_PINKY);

  // Start in "open" (all 0 degrees)
  applyToServos();

  Serial.println("Ready. Send commands like: $00000");
  Serial.println("Mapping: d0=Thumb d1=Index d2=Middle d3=Ring d4=Pinky");
}

void loop()
{
  receiveData();
}
