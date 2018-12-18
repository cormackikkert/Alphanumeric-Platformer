#include "pitches.h"

void setup() {
  Serial.begin(9600);

  short melody[] = {
    NOTE_C6,
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_DS6,
    NOTE_DS6,
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_C6, //
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_DS6,
    NOTE_DS6,
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_C6, //
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_DS6,
    NOTE_DS6,
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_C6, //
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_DS6,
    NOTE_DS6,
    NOTE_C6,
    NOTE_AS5,
    NOTE_C6,
    NOTE_C5, //
    NOTE_D5,
    NOTE_DS5,
    NOTE_F5,
    NOTE_DS5,
    NOTE_F5,
    NOTE_FS5, //
    NOTE_G5,
    NOTE_FS5,
    NOTE_G5,
    NOTE_FS5,
    NOTE_F5,
    NOTE_FS5, //
    NOTE_G5,
    NOTE_FS5,
    NOTE_G5,
    NOTE_FS5,
    NOTE_C5, //
    NOTE_D5,
    NOTE_DS5,
    NOTE_F5,
    NOTE_DS5,
    NOTE_F5,
    NOTE_FS5, //
    NOTE_G5,
    NOTE_FS5,
    NOTE_G5,
    NOTE_FS5,
    NOTE_F5,
    NOTE_FS5, //
    NOTE_G5,
    NOTE_FS5,
    NOTE_G5,
    NOTE_FS5,
    NOTE_C5, //
    NOTE_G5,
    NOTE_C6,
    NOTE_AS5,
    NOTE_AS5,
    NOTE_C6,
    NOTE_G5,
    NOTE_AS5, //
    NOTE_G5,
    NOTE_F5,
    NOTE_DS5,
    NOTE_C5, //
    NOTE_G5,
    NOTE_C6,
    NOTE_AS5,
    NOTE_AS5,
    NOTE_C6,
    NOTE_G5,
    NOTE_AS5, //
    NOTE_G5,
    NOTE_F5,
    NOTE_F5,
    NOTE_C5, //
    NOTE_G5,
    NOTE_C6,
    NOTE_AS5,
    NOTE_AS5,
    NOTE_C6,
    NOTE_G5,
    NOTE_AS5, //
    NOTE_G5,
    NOTE_F5,
    NOTE_DS5,
    NOTE_C5, //
    NOTE_G5,
    NOTE_C6,
    NOTE_AS5,
    NOTE_AS5,
    NOTE_C6,
    NOTE_G5,
    NOTE_AS5, //
    NOTE_F5,
    NOTE_C5,
    NOTE_C5
  };
  byte durations[] = {
    2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 1, 2, 6, 6, 6, 6, 4, 4, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 4, 6, 6, 6, 6, 4, 4, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 4, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2
  };

  for (byte i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    Serial.print("melody[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(melody[i]);
    Serial.println(";");
  }

  for (byte i = 0; i < sizeof(durations) / sizeof(durations[0]); i++) {
    Serial.print("durations[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(durations[i]);
    Serial.println(";");
  }

}

void loop() {
}
