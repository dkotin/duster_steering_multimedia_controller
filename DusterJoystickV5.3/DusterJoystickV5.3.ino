// by Shizlgar для "круглого" джойстика, на основе:
//https://www.drive2.ru/l/6899697/// источник
//https://github.com/Chris--A/Keypad // библиотека
#include <Keypad.h>
// PINOUTS ::::::::::::::::::::::::::::::::::::::::::::::
//
// joystick pins seems like this, lets define pin numbers
//           ** [ 2 6 3 ] **************
//           ** [ 4 5 1 ] ***********
const byte JOY_1 = 7;
const byte JOY_2 = 2;
const byte JOY_3 = 3;
const byte JOY_4 = 4;
const byte JOY_5 = 5;
const byte JOY_6 = 6;


const byte OUT_SEEKUP      = 1;  //0001 //E
const byte OUT_VOLUMEMINUS = 2;  //0010 //v
const byte OUT_SOURCEAUDIO = 3;  //0011 //S
const byte OUT_MODE        = 4;  //0100 //M
const byte OUT_SEEKDOWN    = 5;  //0101 //e
const byte OUT_VOLUMEPLUS  = 6;  //0110 //V
const byte RADIO           = 7;  //0111 //P
const byte MUTE            = 8;  //1000 //X

// TIMOUTS and LEVELS ::::::::::::::::::::::::::::::::::::::::::
int KeyPressedMS = 90;
#define Prog_Time   2500
// pause between key strokes
const int KeyPauseMS = 90;
// loops count between repeats while V or v keys HOLD
const int HoldKeyLoopMax = 5000;

// variables
const byte ROWS = 3;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'v', 'M', 'V'}, // volume down, Mode, Volume up
  {'S', 'P', 'X'}, // Source audio, Radio, Mute
  {'1', '2', '3'}  // encoder as keys
};
byte rowPins[ROWS] = {JOY_4, JOY_5, JOY_6}; // Joy 4 5 6
byte colPins[COLS] = {JOY_1, JOY_3, JOY_2}; // Joy 1 3 2
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char lastEncodeValue = '#';
char keyPressed = '#';
int HoldKeyLoop = -1;
byte lastHoldKey = '?';

// uncomment this to enable debug output
#define DEBUG_TO_COMPORT

void setup() {
#ifdef DEBUG_TO_COMPORT
  Serial.begin(9600);
#endif
  kpd.setDebounceTime(1);
  kpd.setHoldTime(300);
}

void loop() {
#ifdef DEBUG_TO_COMPORT
  simKey_Serial();
#endif
  // Hold-retry 'V' and 'v' keys
  if (HoldKeyLoop == 0) {
    ProcessKeyPressed(lastHoldKey);
    HoldKeyLoop = HoldKeyLoopMax;
#ifdef DEBUG_TO_COMPORT
    Serial.print("Key ");
    Serial.print( lastHoldKey );
    Serial.println(" retry.");
#endif
  }
  if (HoldKeyLoop > 0) {
    HoldKeyLoop = HoldKeyLoop - 1;
  };


  // read keyboard, and if there is a new key event...
  if (kpd.getKeys())
  {
    // Scan the whole key list.
    for (int i = 0; i < LIST_MAX; i++)
    {
      if (  millis() < 1000) { // if "МОDE (OK)" button pressed in first second after powerup - enable learning (long presses)
        if ( kpd.key[i].kchar == 'M') {
          KeyPressedMS = Prog_Time;
        }
      }
      keyPressed = '?';

      // if any key release, stop HOLD key retries
      if (kpd.key[i].kstate == RELEASED) {
        HoldKeyLoop = -1;
#ifdef DEBUG_TO_COMPORT
        //    Serial.println("Some key released.");
#endif
      };

      // Handle HOLD 'v' and 'V' keys event - start to do series
      if ( ( kpd.key[i].kchar == 'V' || kpd.key[i].kchar == 'v') && kpd.key[i].kstate == HOLD	)
      {
        keyPressed = kpd.key[i].kchar;
        HoldKeyLoop = HoldKeyLoopMax;
        lastHoldKey = keyPressed;
        ProcessKeyPressed(keyPressed);
#ifdef DEBUG_TO_COMPORT
        Serial.print("Key ");
        Serial.print( keyPressed );
        Serial.println(" hold.");
#endif
      }

      // find keys that have changed state and it's new state is PRESSED.
      if ( kpd.key[i].stateChanged && kpd.key[i].kstate == PRESSED)
      {
        keyPressed = kpd.key[i].kchar;
        if (keyPressed == '1' || keyPressed == '2' || keyPressed == '3') {
          // encoder decode
          if (  lastEncodeValue == '#' ) lastEncodeValue = keyPressed;
          if ( (lastEncodeValue == '1' && keyPressed == '2' ) ||
               (lastEncodeValue == '2' && keyPressed == '3' ) ||
               (lastEncodeValue == '3' && keyPressed == '1' ) ) {
            lastEncodeValue = keyPressed;
            keyPressed = 'e'; // down
          } else if ((lastEncodeValue == '1' && keyPressed == '3' ) ||
                     (lastEncodeValue == '2' && keyPressed == '1' ) ||
                     (lastEncodeValue == '3' && keyPressed == '2' ) ) {
            lastEncodeValue = keyPressed;
            keyPressed = 'E'; // up
          } else {
            keyPressed = '?'; // strange message from encoder
          }
        }
        ProcessKeyPressed(keyPressed);
#ifdef DEBUG_TO_COMPORT
        Serial.print("Key= ");
        Serial.println( keyPressed );
        //   Serial.println(" pressed.");
#endif
      }
    } // end for eash new key event/ завершение для каждого нового ключевого события
  } // end if event exist// конца, если событие существует

}  // End main loop// Конец основного цикла

// process key event: send keystroke to output
void ProcessKeyPressed(byte keyPressed1) {
  // process output
  switch (keyPressed1) {
    case 'S': simKey(OUT_SOURCEAUDIO); break;
    case 'V': simKey(OUT_VOLUMEPLUS) ; break;
    case 'v': simKey(OUT_VOLUMEMINUS); break;
    case 'P': simKey(RADIO); break;
    case 'X': simKey(MUTE); break;
    case 'E': simKey(OUT_SEEKUP);   break;
    case 'e': simKey(OUT_SEEKDOWN); break;
    case 'M': simKey(OUT_MODE);     break;
  }
}

// simulate one key press
void simKey(byte leg) {
  DDRB = DDRB | leg;
  PORTB &= leg;
  digitalWrite(13, HIGH);
  delay(KeyPressedMS);
  for (byte i = 8; i <= 11; i++) {
    pinMode(i, INPUT);
  }
  digitalWrite(13, LOW);
  delay(KeyPauseMS);
}

#ifdef DEBUG_TO_COMPORT
void simKey_Serial() {
  if (Serial.available() ) {
    String  Str_Buff = "";
    Str_Buff = Serial.readString();
    byte  CAP =  Str_Buff.toInt();

    if (CAP >= 0 && CAP <= 15) {
      simKey(CAP);
      Serial.print("OK,OUT_CAP = "); Serial.println(CAP);
    }
    else Serial.println("HET!  OT 0 DO 15");
  }
}
#endif
