// last update 05-мая-2018
// 24/apr/2018 = door relays checked and updated  LOW = door closed & TEAMVIEWER CORRRECTED

// test run, all buttons and light function
//Тоннель 2.0 (работает, проходим игру до конца, свет исправили, ошибку отрабатывает правильно)
//Тоонель оборудован двумя дверьми с двух сторон. Каждая дверь открывается двумя кнопками (изнутри и снаружи). Снаружи тоннель оборудован монитором, отображающий информацию с видеокамреы, установленной внутри (вид на вход). Внутри тоннеля на полу расположены 12 плиток-кнопок (датчики давления) в два ряда по 6 штук в длинну. На полу есть подсветка, в тоннеле музыкальное сопровождение.
//Алгоритм:
//Играет один человек. Для запуска игры нажимаем кнопку открывания 1 двери снаружи.
//Игрок должен встать на 1 клетку.
//#include "Arduino.h"
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

//Глобальные переменные:
#define REDRELAY    35
#define BLURELAY    33
#define GRERELAY    31
#define STARTPIN    37
#define DOOR1RELAY  27
#define DOOR2RELAY  29
#define DOOR1SENS   23
#define DOOR2SENS   25
#define DOOR1BUTIN  41
#define DOOR1BUTOUT 43
#define DOOR2BUTIN  45
#define DOOR2BUTOUT 47
#define FLOORLEDPIN 53
#define DIRLEDPIN   49
#define STEPLEDPIN  51
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
#define ONETIMEPRESSEDPERIOD 300
#define STEPTIME 15000

//Переменные состояний:
boolean door1Closed = true;
boolean door2Closed = true;
boolean door1ButtOutState[2] = {HIGH, HIGH};
boolean door1ButtInState[2] = {HIGH, HIGH};
boolean door2ButtOutState[2] = {HIGH, HIGH};
boolean door2ButtInState[2] = {HIGH, HIGH};
boolean door1SensorState[2] = {LOW, LOW};
boolean door2SensorState[2] = {LOW, LOW};
boolean startPinState[2] = {HIGH, HIGH};
boolean startSignal = true;
boolean GameDone = false;
boolean game = true;
boolean cell_passed = false;
boolean blinkState = false;
boolean greenorblue = false;
boolean TEST = true;
unsigned long nowTime = 0;
unsigned long d2ButInPressed = 0;
unsigned long d2ButOutPressed = 0;
boolean simpleorhard = false;
int x, y, prev_x, prev_y;
int buttPins[10] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};
int ledByCell = 8; //Количество светодиодов на одну клетку
int ledByDir = 1; //Количество светодиодов на одно направление
int cellCount = 10; //Количество клеток на полу
int floorLedCount = 80; // Общее количество светодиодов подсветки клеток = 80
int dirPanelLedCount = 8; // 8 на указатель направления (в реальности 4 + ledByDir * 1 = 8) !!!!! Поменять
int currStepIndexLedCount = 8; // 8 на индикатор шагов.
int playersPassed = 0;

int r = 0;
int s = 0;
int level = 0;
int playerCount = 0;
CRGB wsLeds[180];
CRGB darkColor, whiteColor, greenColor, redColor, blueColor;
int routes[4][9] = {
  {0, 5, 6, 7, 8, 3, 3, 4, 9},
  {5, 6, 7, 2, 1, 1, 2, 3, 4},
  {5, 6, 1, 2, 7, 2, 3, 3, 4},
  {0, 1, 2, 7, 7, 8, 3, 8, 9}
};
//-0-1-2-3-4-
//-9-8-7-6-5-

//-0-1-2-3-4-
//-5-6-7-8-9-
//SendOnlySoftwareSerial mp3Serial(2);


void setup()
{
  Serial.begin(9600);
  //  Serial2.begin(9600);
  mp3_set_serial (Serial);
  delay(10);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (28);
  delay(100);

  Serial.println("\n Tunnel Start \n");
  delay(100);

  pinMode(FLOORLEDPIN, OUTPUT);
  pinMode(DIRLEDPIN, OUTPUT);
  pinMode(STEPLEDPIN, OUTPUT);

  FastLED.addLeds<LED_TYPE, FLOORLEDPIN, COLOR_ORDER>(wsLeds, 0, floorLedCount).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DIRLEDPIN, COLOR_ORDER>(wsLeds, floorLedCount, dirPanelLedCount).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, STEPLEDPIN, COLOR_ORDER>(wsLeds, floorLedCount + dirPanelLedCount, currStepIndexLedCount).setCorrection(TypicalLEDStrip);

  redColor = CRGB(0, 100, 0);
  whiteColor = CRGB(10, 10, 10);
  greenColor = CRGB(100, 0, 0);
  blueColor = CRGB(0, 0, 100);
  darkColor = CRGB(0, 0, 0);

  for (int c = 0; c < 80; c++) wsLeds[c] = whiteColor;
  FastLED.show();
  delay(500);
  for (int c = 0; c < 80; c++) wsLeds[c] = darkColor;
  FastLED.show();
  Serial.println("Floor leds test OK");
  for(int c = 80; c < 88; c++)
  {
    wsLeds[c] = whiteColor;
    FastLED.show();
    delay(400);
    wsLeds[c] = darkColor;
    FastLED.show();
  }
  Serial.println("Direction leds test OK");
  for(int c = 88; c < 96; c++)
  {
    wsLeds[c] = whiteColor;
    FastLED.show();
    delay(400);
    wsLeds[c] = darkColor;
    FastLED.show();
  }
  Serial.println("Step leds test OK");
  Serial.println("Light Test OK");

  pinMode(STARTPIN, INPUT_PULLUP);
  pinMode(DOOR1SENS, INPUT_PULLUP);
  pinMode(DOOR2SENS, INPUT_PULLUP);
  pinMode(DOOR1BUTIN, INPUT_PULLUP);
  pinMode(DOOR1BUTOUT, INPUT_PULLUP);
  pinMode(DOOR2BUTIN, INPUT_PULLUP);
  pinMode(DOOR2BUTOUT, INPUT_PULLUP);

  pinMode(DOOR1RELAY, OUTPUT);
  pinMode(DOOR2RELAY, OUTPUT);

  pinMode(REDRELAY, OUTPUT);
  pinMode(BLURELAY, OUTPUT);
  pinMode(GRERELAY, OUTPUT);
  //  pinMode(YELRELAY, OUTPUT);
  
  while(digitalRead(DOOR2SENS)){;}
  digitalWrite(DOOR2RELAY, LOW);
  Serial.println("Door B Closed");
  while(digitalRead(DOOR1SENS)){;}
  digitalWrite(DOOR1RELAY, LOW);
  Serial.println("Door A Closed");
  if(!digitalRead(DOOR1BUTOUT)) 
  {
    simpleorhard = true;
    Serial.println("Welcome to hard style...");
  }
  for (int i = 0; i < 10; i++) pinMode(buttPins[i], INPUT_PULLUP);

  randomSeed(A0);
  digitalWrite(GRERELAY, HIGH);
  digitalWrite(REDRELAY, HIGH);
  digitalWrite(BLURELAY, HIGH);
  Serial.println("OFF OK");
  LightsTest();
  Serial.print("\n Level =");
  Serial.println(level);
  delay(100);
  mp3_play(23);
}

void loop()
{
  if (level == 1000)
  {
    fbTest();
    butTest();
  }
  if (level == 0) // Ждем сигнала СТАРТ
  {
    
    startPinState[0] = debounce(startPinState[1], STARTPIN);
    if (!startPinState[0] && startPinState[1]) restartLevel();
    if (digitalRead(DOOR1BUTOUT) == LOW) restartLevel();
    startPinState[1] = startPinState[0];
  }
  else if (level == 1) // Ждем захода игрока внутрь.
  {
    if (door1Closed)
    {
      door1ButtOutState[0] = debounce(door1ButtOutState[1], DOOR1BUTOUT); // Ждем нажатия наружной кнопки двери А.
      if (!door1ButtOutState[0] && door1ButtOutState[1])
      {
        //door1Closed = false;
        digitalWrite(DOOR1RELAY, HIGH); // Открываем дверь
        digitalWrite(BLURELAY, LOW); // включаем синий цвет
        Serial.println("DOOR 1 OUT BUTTON PRESSED"); // Отладочная строка
        door1SensorState[1] = digitalRead(DOOR1SENS);
        /*while(!door1Closed) // Ждем открытия двери рукой (сработки сенсора)
          {
          door1SensorState[0] = debounce(door1SensorState[1], DOOR1SENS);
          if (!door1SensorState[0] && door1SensorState[1]) door1Closed = false;
          door1SensorState[1] = door1SensorState[0];
          }*/
        while (!digitalRead(DOOR1SENS)) {
          ;
        }
        door1Closed = false;
        cellLight(routes[r][s], blueColor); // Включаем синий цвет на стартовой клетке
        Serial.println("DOOR 1 OPENED BY PLAYER");
        door1ButtOutState[0] = HIGH;
      }
      door1ButtOutState[1] = door1ButtOutState[0];
    }
    else // Ждем закрытия двери А игроком.
    {
      door1SensorState[0] = debounce(door1SensorState[1], DOOR1SENS);
      //Serial.println("Was:"+String(door1SensorState[1])+" Now:"+String(door1SensorState[0]));
      if (!door1SensorState[0] && door1SensorState[1])
      {
        door1Closed = true;
        digitalWrite(DOOR1RELAY, LOW); // Закрывание двери А на замок
        Serial.println("DOOR 1 CLOSED FROM INSIDE");
        //digitalWrite(BLURELAY, LOW); // Выключаем синий свет.
        // digitalWrite(YELRELAY, HIGH);
        level = 2;
        if (playerInside()) // Если игрок внутри (нажата хоть 1 кнопка на полу)
        {
          for (int fb = 0; fb < 10; fb++) // Проверяем на какой клетке стоит игрок после закрывания двери
          {
            if (!digitalRead(buttPins[fb]) && fb != routes[r][s])
            {
              Serial.println("BUTT STAND ERROR");
              for (int c = 0; c < floorLedCount; c++) wsLeds[c] = darkColor;
              cellLight(fb, redColor);
              endGame(); // Если стоит не на той клетке
            }
          }
        }
        else endGame(); // If player stay outside
      }
      door1SensorState[1] = door1SensorState[0];
    }
  }
  else if (level == 2)
  {
    /*
       1.
    */
    unsigned long startStepTime = millis();
    unsigned long nextTime = startStepTime;
    unsigned long blinkTime = startStepTime;
    boolean nextCell = false;
    blinkState = false;
    int period = 1000;
    Serial.println("Curr Cell #" + String(routes[r][s] + 1) + " X=" + String(routes[r][s] / 5) + " Y=" + String(routes[r][s] % 5));
    Serial.println("Next Cell #" + String(routes[r][s + 1] + 1) + " X=" + String(routes[r][s + 1] / 5) + " Y=" + String(routes[r][s + 1] % 5));
    boolean pressedOKBeforeTimeisUp = false;
    while ((nextTime - startStepTime) < STEPTIME)
    {
      nextTime = millis();
      // Check the player step cells
      if (!digitalRead(DOOR1BUTIN))
      {
        endGame(); // If press Door A button from inside...(no more buttons inside)..
      }
      if (!cell_passed)
      {
        if (nextTime - blinkTime > period)
        {
          blinkState = !blinkState;
          blinkTime = nextTime;
          if (period > 300) period -= period / 25;
          if (blinkState) cellLight(routes[r][s], blueColor);
          else cellLight(routes[r][s], darkColor);
        }
        if (!readButtons(r, s))
        {
          endGame();//Wrong way
        }
      }
      else
      {
        cellLight(routes[r][s], darkColor);
        pressedOKBeforeTimeisUp = true;
        break;
      }
    }
    Serial.println("Time is over");
    if(!pressedOKBeforeTimeisUp) cell_passed = readButtons2(r, s);
    if (cell_passed)
    {
      s++;
      for (int i = 0; i < 2; i++)
      {
        cellLight(routes[r][s], darkColor);
        delay(200);
        cellLight(routes[r][s], blueColor);
        delay(200);
      }
      if (s < 8) showPanelUpdate(r, s);
      else
      {
        for (int g = 0; g < floorLedCount; g++) wsLeds[g] = greenColor;
        for (int d = 0; d < dirPanelLedCount; d++) wsLeds[floorLedCount + d] = darkColor;
        wsLeds[95] = whiteColor;
        FastLED.show();
        digitalWrite(GRERELAY, LOW);
        digitalWrite(BLURELAY, HIGH);
        Serial.println("Tunnel passed");
        delay(1000);
        digitalWrite(DOOR2RELAY, HIGH); // Открываем дверь Б
        playersPassed++;
        Serial.println("PlayersPassed="+String(playersPassed));
        if(playersPassed > 1)
        {
          digitalWrite(DOOR1RELAY, HIGH);
          Serial.println("TWO PLAYERS PASSED");
          level = 4;
        }
        delay(10);
        door2Closed = false;
        door2SensorState[0] = true;
        door2SensorState[1] = true;
        level = 3;
      }
    }
    else endGame(); //Time is over and another cell pressed.
    cell_passed = false;
  }
  else if (level == 3) //Выход игрока из двери Б
  {
    if (!door2Closed) // 
    {
      door2SensorState[0] = debounce(door2SensorState[1], DOOR2SENS);
      if (!door2SensorState[0] && door2SensorState[1])
      {
        if (!playerInside())
        {
          digitalWrite(DOOR2RELAY, LOW); // Закрываем дверь Б
          door2Closed = true;
          Serial.println("Player closed door B from outside.");
          Serial.println("Wait for press OutBut1 to open door A");
          
        }
        else
        {
          Serial.println("PLEASE, GET OUT FROM TONNEL THROW DOOR B");
        }
      }
      door2SensorState[1] = door2SensorState[0];
    }
    else // Ждем открытия двери А следующим игроком.
    {
      //prepare to game again with next player...
      restartLevel();
    }
  }
  else if (level == 4) // Конец игры.
  {
    /*
      Up splash
      green -------__------__------
      blue  -----------__------__--
      Down splash
      several of 10 cells light up in blue and green color with fade every cycle
    */
    int rand[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int lightCount = 0;
    while (lightCount < 5)
    {
      lightCount = 0;
      for (int i = 0; i < 10; i++)
      {
        rand[i] = random(0, 3);
        if (rand[i] > 0) lightCount++;
      }
    }

    for (int br = 0; br < 255; br++)
    {
      delay(20);
      for (int c = 0; c < 10; c++)
      {
        if (rand[c] == 0) cellLight(c, darkColor);
        else if (rand[c] == 1) cellLight(c, CRGB(br, 0, 0));
        else if (rand[c] == 2) cellLight(c, CRGB(0, 0, br));
      }
      FastLED.show();
    }
    for (int br = 255; br > 0; br--)
    {
      delay(20);
      for (int c = 0; c < 10; c++)
      {
        if (rand[c] == 0) cellLight(c, darkColor);
        else if (rand[c] == 1) cellLight(c, CRGB(br, 0, 0));
        else if (rand[c] == 2) cellLight(c, CRGB(0, 0, br));
      }
      FastLED.show();
    }
    if (blinkState)
    {
      digitalWrite(GRERELAY, HIGH);
      digitalWrite(BLURELAY, HIGH);
    }
    else
    {
      if (greenorblue)
      {
        greenorblue = !greenorblue;
        digitalWrite(GRERELAY, LOW);
      }
      else
      {
        greenorblue = !greenorblue;
        digitalWrite(BLURELAY, LOW);
      }
    }
    blinkState = !blinkState;
  }
}

void showPanelUpdate(int _r, int _s)
{
  //Serial.println("Route = " + String(_r) + " Step = " + String(_s));
  int dir = 0;
  int now_x = routes[_r][_s] / 5;
  int now_y = routes[_r][_s] % 5;
  Serial.println("STEP = " + String(_s));
  Serial.println("P_X = " + String(now_x) + " P_Y = " + String(now_y));
  int new_x = routes[_r][_s + 1] / 5;
  int new_y = routes[_r][_s + 1] % 5;
  Serial.println("N_X = " + String(new_x) + " N_Y = " + String(new_y));
  if (new_y > now_y) {
    dir = 1;  //Вперед
    if(simpleorhard) dir = 0;
    Serial.println("Step Forward, dir = " + String(dir));
  }
  if (new_y < now_y) {
    dir = 3;  //Назад
    if(simpleorhard) dir = 2;
    Serial.println("Step Back, dir = " + String(dir));
  }
  if (new_x < now_x) {
    dir = 0;  //Влево
    if(simpleorhard) dir = 3;
    Serial.println("Step Left, dir = " + String(dir));
  }
  if (new_x > now_x) {
    dir = 2;  //Вправо
    if(simpleorhard) dir = 1;
    Serial.println("Step Right, dir = " + String(dir));
  }
  if(new_x == now_x && new_y == now_y)
  {
    dir = 4;
    Serial.println("Stay this");
  }
  int playerArrow = (6 - _s) % 4;
  int dirArrow = dir;
  if(simpleorhard) dirArrow = (((_s+1)%4)+dir)%4;
  //Serial.println(", OUT OFFSET = " + String(offset * 90));
  for (int l = 0; l < dirPanelLedCount + currStepIndexLedCount; l++) wsLeds[floorLedCount + l] = darkColor; //Тушим
  //int outdir = (dir + offset) % 4;
  for (int o = -1; o < 2; o++) wsLeds[floorLedCount + (4 + o + playerArrow) % 4] = whiteColor; // Нос
  wsLeds[floorLedCount + 4 + dirArrow] = whiteColor;
  //wsLeds[floorLedCount + 4 + ledByDir * outdir] = whiteColor; // Подсветка направления снаружи
  for (int sti = 0; sti < 8; sti++) wsLeds[floorLedCount + dirPanelLedCount + sti] = ((_s > sti) ? whiteColor : darkColor); //Подсветка ИШ
  FastLED.show();
  
  //else Serial.print("STEP = " + String(_s));
}

//================================ CHECK BUTTONS ======================================//
boolean readButtons(int _r, int _s)
{
  for (int i = 0; i < 10; i++)
  {
    if (!digitalRead(buttPins[i]))
    {
      if (i != routes[_r][_s + 1] && i != routes[_r][_s]) //Если стоим не на текущей и не на следующей
      {
        cellLight(i, redColor); // Зажигаем ее крайной
        cellLight(routes[_r][_s], darkColor); // Тушим нужные
        cellLight(routes[_r][_s + 1], darkColor);
        Serial.println("Wrong step");
        return false; // Возвращаемся
      }
      else if (!cell_passed) // Если первый раз наступили на следующую
      {
        if (i == routes[_r][_s + 1])
        {
          Serial.println("Cell Passed OK");
          cell_passed = true;
          cellLight(i, blueColor); // Зажигаем ее зеленой
          cellLight(routes[_r][_s], darkColor); // Предыдущую гасим.
        }
      }
    }
  }
  return true;
}
boolean readButtons2(int _r, int _s)
{
  for (int i = 0; i < 10; i++)
  {
    if (!digitalRead(buttPins[i]))
    {
       if(i != routes[_r][_s + 1])
       {
        cellLight(i, redColor); // Зажигаем ее крайной
        cellLight(routes[_r][_s], darkColor); // Тушим нужные
        cellLight(routes[_r][_s + 1], darkColor);
        Serial.println("Time is up");
        return false;
       }
    }
  }
  return true;
}
//================================ CHECK BUTTONS ======================================//

void cellLight(int _cell, CRGB _color)
{
  if (_cell < 5)
  {
    for (int c = 0; c < ledByCell; c++) wsLeds[_cell * ledByCell + c] = _color;
  }
  else
  {
    for (int c = 0; c < ledByCell; c++) wsLeds[(14 - _cell) * ledByCell + c] = _color;
  }
  FastLED.show();
}
void endGame()
{
  if (playerInside())
  {
    digitalWrite(BLURELAY, HIGH);
    digitalWrite(GRERELAY, HIGH);
    digitalWrite(REDRELAY, LOW);
    for (int dl = 0; dl < dirPanelLedCount + currStepIndexLedCount; dl++) wsLeds[floorLedCount + dl] = redColor;
    FastLED.show();
    digitalWrite(DOOR1RELAY, HIGH);
    while (!digitalRead(DOOR1SENS)) {
      ;// Ждем пока дверь окроется
    }
    Serial.println("DOOR 1 OPENED, WAIT FOR PLAYER GOING OUT AND CLOSE HER");
  }

  while (digitalRead(DOOR1SENS)) {
    ;// Ждем пока дверь закроется
  }
  digitalWrite(DOOR1RELAY, LOW);
  Serial.println("DOOR 1 CLOSED");
  door1Closed = true;
  for (int dl = 0; dl < dirPanelLedCount + currStepIndexLedCount; dl++) wsLeds[floorLedCount + dl] = darkColor; //Гасим свет
  FastLED.show();
  digitalWrite(REDRELAY, HIGH); //Верхний тоже
  Serial.println("RED LIGHT OFF");
  cell_passed = false;
  level = 0;
}

boolean playerInside()
{
  boolean playerIn = false;
  for (int b = 0; b < 10; b++) if (!digitalRead(buttPins[b])) playerIn = true;
  if (playerIn) Serial.println("PLAYER INSIDE");
  else Serial.println("PLAYER OUTSIDE");
  return playerIn;
}

boolean debounce(boolean prevstate, int pin)
{
  boolean currstate = digitalRead(pin);
  if (currstate != prevstate)
  {
    delay(5);
    currstate = digitalRead(pin);
  }
  return currstate;
}

void restartLevel()
{
  level = 1;
  r = random(0, 4);
  Serial.println("Route#" + String(r));
  for (int i = 0; i < 9; i++)
  {
    Serial.print("-" + String(routes[r][i]));
  }
  Serial.println("-");
  s = 0;
  showPanelUpdate(r, s);
  for (int g = 0; g < floorLedCount; g++) wsLeds[g] = darkColor;
  FastLED.show();
  digitalWrite(REDRELAY, HIGH);
  digitalWrite(BLURELAY, LOW); //ON
  digitalWrite(GRERELAY, HIGH);
}

void checkTwoPushes()
{
  nowTime = millis();
  door2ButtOutState[0] = debounce(door2ButtOutState[1], DOOR2BUTOUT);
  door2ButtInState[0] = debounce(door2ButtInState[1], DOOR2BUTIN);
  if (!door2ButtOutState[0] && door2ButtOutState[1]) d2ButOutPressed = nowTime;
  if (!door2ButtInState[0] && door2ButtInState[1]) d2ButInPressed = nowTime;
  if (d2ButInPressed > 0)
  {
    if (d2ButOutPressed > 0)
    {
      if (d2ButOutPressed - d2ButInPressed > 0 && d2ButOutPressed - d2ButInPressed < ONETIMEPRESSEDPERIOD
          || d2ButInPressed - d2ButOutPressed > 0 && d2ButInPressed - d2ButOutPressed < ONETIMEPRESSEDPERIOD)
      {
        level = 5;
        digitalWrite(DOOR1RELAY, LOW);
        digitalWrite(DOOR2RELAY, LOW);
        while (!digitalRead(DOOR1SENS)) {
          ;
        }
        while (!digitalRead(DOOR2SENS)) {
          ;
        }
        Serial.println("WIN");
      }
    }
    else
    {
      door2Closed = false;
      digitalWrite(DOOR2RELAY, HIGH);
      Serial.println("DOOR 2 UNLOCKED");
      delay(500);
      while (!digitalRead(DOOR2SENS)) {
        ; // Ждем пока дверь Б откроется (страховка в случае когда пружинка не сработает)
      }
      Serial.println("DOOR 2 OPENED");
      delay(100);
      while (digitalRead(DOOR2SENS)) {
        ; // Ждем пока игрок закроет дверь Б снаружи...
      }
      delay(100);
      digitalWrite(DOOR2RELAY, HIGH);
      Serial.println("DOOR 2 CLOSED AND LOCKED");

      d2ButOutPressed = 0;
      d2ButInPressed = 0;
    }
  }
  door2ButtOutState[1] = door2ButtOutState[0];
  door2ButtInState[1] = door2ButtInState[0];
}

