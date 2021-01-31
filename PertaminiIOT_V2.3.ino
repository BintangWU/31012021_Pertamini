#include <EEPROM.h>
#include <Keypad.h>

#define ROWS          4
#define COLS          3

const char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

const uint8_t rowPins[ROWS] = {7, 8, 9, 10};
const uint8_t colPins[COLS] = {11, 12, A1};
char key;
Keypad        keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#include "LedControl.h"
#define nDisplay      3   //Jumlah Modul 7segment MAX7219
LedControl    led = LedControl(4, 6, 5, nDisplay); //DataIn, CLK, CS

#define flowPin       2   //Pin flow sensor
#define nozlePin      3   //Pin switch nozle

#define Pump          A3
#define Valve         A2

#include "Fungsi.h"

void setup() {
  Serial.begin(115200);

  keypad.addEventListener(keypadEvent);

  pinMode(flowPin, INPUT);
  pinMode(nozlePin, INPUT);

  pinMode(Pump, OUTPUT);
  pinMode(Valve, OUTPUT);

  for (cnt = 0; cnt < nDisplay; cnt++) {
    led.shutdown(cnt, false);
    led.setIntensity(cnt, 15);
    led.clearDisplay(cnt);
  }

  attachInterrupt(digitalPinToInterrupt(flowPin), count, RISING);
  cnt = 0;
  cntMenu = 0;
  //faktor = 255;
  EEPROM.get(4, cost);
  EEPROM.get(0, faktor);
  // load(); //LOAD EEPROM
}

void loop() {
  key = keypad.getKey();
  switch (cntMenu) {
    case 0  : //Ready to run
      //Run
      if (run) {
        clear();

        testDisplay();
        //digitalWrite(Valve, HIGH);
        digitalWrite(Pump, HIGH); //Pump ON
        delay(500);

        pulse = 0;
        costCnt = 0;
        literCnt = 0;
        showDefault();
        delay(500);

        attachInterrupt(digitalPinToInterrupt(flowPin), count, RISING);

        do {
          showCount(2, costCnt, 3);     //Total harga
          showCount(1, literCnt, 2);    //Total liter yang dikeluarkan
          showCount(0, cost, 3);        //Harga per Liter

          if ((costCnt >= costLimit) && (literCnt >= literLimit)) {
            status =  false;
            detachInterrupt(digitalPinToInterrupt(flowPin));

            if (costLimit != 0) {
              costCnt = costLimit;
              if (costLimit % cost == 0)
                literCnt = 100 * (costLimit / cost);
            }

            if (literLimit != 0) {
              literCnt = literLimit;
              if ((int)literLimit % 2 == 0)
                costCnt  = cost * (literLimit / 100);
            }
          }
          else {
            status = true;

            if (keypad.getKey() == '*') {
              status = false;
              detachInterrupt(digitalPinToInterrupt(flowPin));
            }
          }
        }
        while (status);
        //digitalWrite(Valve, LOW);
        digitalWrite(Pump, LOW);//Pump ON
        pulse = 0;

        while (!status) {
          showCount(2, costCnt, 3);    //Total Rupiah
          showCount(1, literCnt, 2);   //Total Liter yang dikeluarkan
          showCount(0, cost, 3);      //Harga per

          if (keypad.getKey() == '*') {
            clear();
            delay(10);
            break;
          }
        }
        run = false;
        status = false;
        clear();
      }
      else {
        //Tampilan awal
        showDefault();
        detachInterrupt(digitalPinToInterrupt(flowPin));
      } break;

    case 1  : //Set. beli dgn jumalah harga Rp.-----
      showCount(2, costLimit, 3);
      if (key >= '0' && key <= '9') {
        temp += key;
        costLimit = temp.toInt();
      } break;

    case 2  : //Set. beli dgn jumalah ----- liter
      showCount(1, literLimit, 2);
      if (key >= '0' && key <= '9') {
        temp += key;
        literLimit = temp.toInt();
      } break;

    case 3 : //Set. pengaturan harga per liter
      showCount(2, newCost, 3);
      showCount(0, cost, 3);
      if (key >= '0' && key <= '9') {
        temp += key;
        newCost = temp.toInt();
      } break;

    case 4 : //Set. kalibrasi
      //Rumus = newFaktor = (Faktor * BBM terukur) / BBM
      led.setChar(2, 7, 'o', false);
      led.setChar(1, 7, 'n', false);

      showCount(2, faktor, 2);
      showCount(1, newFaktor, 2);

      if (key >= '0' && key <= '9') {
        temp += key;
        newFaktor = temp.toInt();
      } break;
  }
}

void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case PRESSED  :
      switch (key) {
        case '*'  :
          clear();

          if (cntMenu == 0)
            cntMenu = 1;
          else if (cntMenu == 1)
            cntMenu = 2;
          else if (cntMenu == 2 || cntMenu == 5)
            cntMenu = 0;
          else if (cntMenu == 3)
            cntMenu = 4;
          else if (cntMenu == 4)
            cntMenu = 3;

          costLimit = 0;
          literLimit = 0.00;
          pulse = 0;
          newCost = 0;
          clear();

          if (cntMenu == 2 || cntMenu == 4) {
            for (uint8_t i = 0; i < 3; i++)
              showCount(1, 0, 2, i);
          } break;
      }

    case HOLD     :
      switch (key) {
        case '*' : //Masuk ke pengaturan harga per liter dan kalibrasi
          if (cntMenu < 3)
            cntMenu = 3;
          else
            cntMenu = 0;

          newCost = 0;
          clear();
          break;

        case '#' :
          //Set. dan save harga baru
          if (newCost > 0) {
            cost = newCost;
            EEPROM.put(4, cost);
            //save(); //SAVE EEPROM
            cntMenu = 0;
            newCost = 0;
            clear();
          }

          if (newFaktor > 0) {
            faktor = newFaktor;
            EEPROM.put(0, faktor);
            cntMenu = 0;
            newFaktor = 0;
            clear();
          } break;
      }
  }
}


