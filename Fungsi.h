bool run = false;
bool status = false;
bool flag = false;


String temp = "";
uint8_t cnt, cntMenu;

long pulse, newCost;
long faktor, newFaktor; //2.50;

long costCnt = 0, costLimit = 0, cost;
float literCnt = 0.00, literLimit = 0.00;

void count() {
  pulse += 1;
  if (faktor <= 0)
    faktor = 255;
  literCnt = (pulse * (faktor / 100)) / 10;
  costCnt = cost * (literCnt / 100);
}

void showCount(uint8_t nDisp, long number, uint8_t dp, uint8_t pos = 0) {
  uint8_t digit = number % 10;
  long remainDigits = number / 10;

  led.setDigit(nDisp, pos, digit, pos == dp ? true : false);
  if (remainDigits > 0)
    showCount(nDisp, remainDigits, dp, pos + 1);
}

void showDefault() {
  showCount(2, 0, 3);
  for (uint8_t i = 0; i < 3; i++)
    showCount(1, 0, 2, i);
  showCount(0, cost, 3);
}

void testDisplay() {
  for (uint8_t i = 0; i < 2; i++) {
    for (cnt = 0; cnt < nDisplay; cnt++) {
      if (i == 0)
        led.test(cnt, true);
      else
        led.test(cnt, false);
    }
    delay(1000);
  }
}

void tampil(long data1, long data2, long data3, uint8_t a, uint8_t b, uint8_t c) {
  showCount(0, data1, a);
  showCount(1, data2, b);
  showCount(2, data3, c);
}

void clear() {
  for (uint8_t i = 0; i < nDisplay; i++)
    led.clearDisplay(i);
  temp = "";
  delay(10);
}

void save() {
  EEPROM.put(0, cost);
  EEPROM.put(4, faktor);
}

void load() {
  EEPROM.get(0, cost);
  EEPROM.get(4, faktor);
}


