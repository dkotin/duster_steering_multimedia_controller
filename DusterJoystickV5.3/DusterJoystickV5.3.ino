// by Shizlgar для "круглого" джойстика, на основе:
//https://www.drive2.ru/l/6899697/// источник
//https://github.com/Chris--A/Keypad // библиотека
#include <Keypad.h>
// PINOUTS ::::::::::::::::::::::::::::::::::::::::::::::
//
// joystick pins seems like this, lets define pin numbers
// штыри джостика кажутся как это, позволяют определить номера штыря
//           ** [ 2 6 3 ] **************
//           ** [ 4 5 1 ] ***********
/// разьем джост //номер пина ардуино//
const byte JOY_1 = 7;//
const byte JOY_2 = 2;
const byte JOY_3 = 3;
const byte JOY_4 = 4;
const byte JOY_5 = 5;
const byte JOY_6 = 6;


//----------------- Ц А П ----------------------------------------------------------------------------
// пример "OUT_VOLUMEMINUS = 2;  //0010 //v"-- значит активный выход 9//на нём ноль//8,10 и11 пины входы
//-----------------------  десят//бинарн//символ
const byte OUT_SEEKUP      = 1;  //0001 //E
const byte OUT_VOLUMEMINUS = 2;  //0010 //v
const byte OUT_SOURCEAUDIO = 3;  //0011 //S
const byte OUT_MODE        = 4;  //0100 //M
const byte OUT_SEEKDOWN    = 5;  //0101 //e
const byte OUT_VOLUMEPLUS  = 4;  //0110 //V
const byte ON_PHONE        = 7;  //0111 //P
const byte OFF_PHONE       = 8;  //1000 //P

// TIMOUTS and LEVELS ::::::::::::::::::::::::::::::::::::::::::
// время активнисти пина , имитация нажатия
int KeyPressedMS = 120;      // время имитации нажатой кнопки
#define Prog_Time   3000      // время имитации нажатой кнопки для программирования
// how long pause between key stroke
const int KeyPauseMS = 100;   // пауза после имитации нажатия
// loops count between repeats while V v keys HOLD
const int HoldKeyLoopMax = 5000;

// variables
const byte ROWS = 3;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'v', 'M', 'V'}, // volume down, Mode, Volume up
  {'S', 'P', 'X'}, // Source audio, Phone, Impossible button
  {'1', '2', '3'}  // encoder as keys
};
byte rowPins[ROWS] = {JOY_4, JOY_5, JOY_6}; // Joy 4 5 6
byte colPins[COLS] = {JOY_1, JOY_3, JOY_2}; // Joy 1 3 2
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char lastEncodeValue = '#';
char keyPressed = '#';
int HoldKeyLoop = -1;
byte lastHoldKey = '?';
bool TELEFON = 0;

// раскомментируйте эту строку, если хотите увидеть отладочный вывод и определить коды клавиш.
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
  simKey_Serial();    // функция имитации ЦАП через печать в порт
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
  // чтение клавиатуры, и если есть новое ключевое событие...
  if (kpd.getKeys())
  {
    // Scan the whole key list.
    for (int i = 0; i < LIST_MAX; i++)
    {
      if (  millis() < 1000) {         // в первую секунду включени ардуино
        if ( kpd.key[i].kchar == 'M') { // если нажата кнопка "МОDE"
          KeyPressedMS = Prog_Time; // увеличиваем время работы кнопок
        }
      }
      keyPressed = '?';

      // if any key release, stop HOLD key retries
      // при отпускании ключа прекратите попытки удержания ключа
      if (kpd.key[i].kstate == RELEASED) {
        HoldKeyLoop = -1;
#ifdef DEBUG_TO_COMPORT
        //    Serial.println("Some key released.");
#endif
      };

      // Handle HOLD 'v' and 'V' keys event - start to do series
      // 'v' and 'V' серия повторов при удержании кнопки
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
      // найти ключи, которые изменили состояние, и это новое состояние нажата.
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
            keyPressed = '?'; // strange message from encoder// странное сообщение от кодировщика
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
// событие ключа процесса: отправить нажатие клавиши на вывод
void ProcessKeyPressed(byte keyPressed1) {
  // process output
  switch (keyPressed1) {
    case 'S': simKey(OUT_SOURCEAUDIO); break;
    case 'V': simKey(OUT_VOLUMEPLUS) ; break;
    case 'v': simKey(OUT_VOLUMEMINUS); break;
    case 'P': simKey(ON_PHONE); break;
    case 'X': simKey(OFF_PHONE); break;
    case 'E': simKey(OUT_SEEKUP);   break;
    case 'e': simKey(OUT_SEEKDOWN); break;
    case 'M': simKey(OUT_MODE);     break;
  }
}

// simulate one key press// имитация нажатия одной клавиши
void simKey(byte leg) {
  DDRB = DDRB | leg;     // назначаем  выходы
  PORTB &= leg;          // устанавливаем  LOW на обьявленные выходы
  digitalWrite(13, HIGH);
  delay(KeyPressedMS);
  for (byte i = 8; i <= 11; i++) {
    pinMode(i, INPUT); // устанавливаем как входа
  }
  digitalWrite(13, LOW);
  delay(KeyPauseMS);
}
//-------------имация ЦАП через порт -------------------------------------
#ifdef DEBUG_TO_COMPORT
void simKey_Serial() {
  if (Serial.available() ) {         // если в порту что то есть
    String  Str_Buff = "";
    Str_Buff = Serial.readString(); // читаем буфер порта
    byte  CAP =  Str_Buff.toInt();  // конвертирование строки  в int

    if (CAP >= 0 && CAP <= 15) {     // если число укладывается в наш диапозон
      simKey(CAP);                  // активируем ЦАП
      Serial.print("OK,OUT_CAP = "); Serial.println(CAP);
    }
    else Serial.println("HET!  OT 0 DO 15"); // предупреждаем о диапозоне ЦАП
  }
}
#endif
