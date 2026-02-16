#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// SH1106 128x64 I2C - Init display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

const int LED_PIN = 23;  // HDD LED pin
const int TOUCH_PIN = 4; // TTP223 Touch Sensor (Signal)

int lastTouchState = LOW;

// ==========================================
// IBM LOGO BITMAP (72x34 Pixel)
// ==========================================
const unsigned char ibm_logo_bits[] U8X8_PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfc, 0x7f, 0xfe, 0xff, 0x07, 0xff, 0x03, 0xf0, 0x3f,
    0xfc, 0x7f, 0xfe, 0xff, 0x0f, 0xff, 0x07, 0xf8, 0x3f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfc, 0x7f, 0xfe, 0xff, 0x3f, 0xff, 0x0f, 0xfc, 0x3f,
    0xfc, 0x7f, 0xfe, 0xff, 0x3f, 0xff, 0x1f, 0xfe, 0x3f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x07, 0xe0, 0x83, 0x3f, 0xf0, 0x3f, 0xfe, 0x03,
    0xc0, 0x07, 0xe0, 0x83, 0x1f, 0xf0, 0x7f, 0xff, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x07, 0xe0, 0xff, 0x0f, 0xf0, 0x7f, 0xff, 0x03,
    0xc0, 0x07, 0xe0, 0xff, 0x07, 0xf0, 0xfd, 0xef, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x07, 0xe0, 0xff, 0x07, 0xf0, 0xfd, 0xef, 0x03,
    0xc0, 0x07, 0xe0, 0xff, 0x0f, 0xf0, 0xf9, 0xe7, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x07, 0xe0, 0x83, 0x1f, 0xf0, 0xf9, 0xe3, 0x03,
    0xc0, 0x07, 0xe0, 0x83, 0x3f, 0xf0, 0xf1, 0xe1, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfc, 0x7f, 0xfe, 0xff, 0x3f, 0xff, 0xf1, 0xe1, 0x3f,
    0xfc, 0x7f, 0xfe, 0xff, 0x3f, 0xff, 0xe1, 0xe0, 0x3f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x00, 0xfc, 0x7f, 0xfe, 0xff, 0x0f, 0xff, 0x41, 0xe0, 0x3f,
    0xfc, 0x7f, 0xfe, 0xff, 0x07, 0xff, 0x41, 0xe0, 0x3f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Enum to group and cycle the demo modes.
enum DemoMode
{
  DEMO_LOGO,
  DEMO_BIOS,
  DEMO_BOOT,
  DEMO_WORDSTAR,
  DEMO_MATRIX,
  DEMO_PONG,
  DEMO_SNAKE
};
// Set initial demo to start on (IBM Logo)
DemoMode mode = DEMO_LOGO;

unsigned long lastSwitch = 0;
int demoDuration = 5000; // ms per demo

// Cursor blink
bool cursor = true;
unsigned long lastBlink = 0;

// IBM 5150 BIOS state
unsigned long biosStart = 0;
bool biosInit = false;
int memKB = 0;

// WordStar typing state
const char *wsText[] = {
    "This is WordStar on ESP32.",
    "Running inside a demo loop.",
    "",
    "The quick brown fox jumps",
    "over the lazy dog.",
    "",
    "Retro text lives forever."};
int wsLine = 0;
int wsChar = 0;
unsigned long lastType = 0;

// Matrix State
const int M_COLS = 16;
const int M_LEN = 8;
struct MatrixDrop
{
  float y;
  float speed;
  int length;
  int chars[M_LEN];
};

MatrixDrop drops[M_COLS];
int getRandomGlyph() { return (random(0, 10) > 2) ? random(12448, 12530) : random(12448, 12530); }

// Pong state
float ballX, ballY, dirX, dirY, p1Y, p2Y;
int score1, score2;
const int PW = 3, PH = 12;

// Snake state
const int GRID_SIZE = 4;
const int GRID_W = 128 / GRID_SIZE;
const int GRID_H = 64 / GRID_SIZE;
struct Point
{
  int x;
  int y;
};

Point snake[100];
int snakeLen = 4;
Point food;
int sDirX = 1, sDirY = 0;

// ================= HELPERS =================

void blinkCursor()
{
  if (millis() - lastBlink > 400)
  {
    lastBlink = millis();
    cursor = !cursor;
  }
}

// ================= IBM LOGO =================
void drawIBMLogo()
{
  u8g2.clearBuffer();
  u8g2.drawXBMP(28, 15, 72, 34, ibm_logo_bits);
  u8g2.sendBuffer();
}

// ================= BIOS =================
void drawBIOS()
{
  if (!biosInit)
  {
    biosInit = true;
    biosStart = millis();
    memKB = 0;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);

  // Top lines exactly like early IBM POST style
  u8g2.drawStr(0, 10, "IBM Personal Computer");
  u8g2.drawStr(0, 18, "");

  // Memory count up animation (classic effect)
  unsigned long t = millis() - biosStart;
  if (t > 200 && memKB < 640)
  {
    memKB += 16; // step size for visual speed
  }

  char memLine[32];
  sprintf(memLine, "%d KB OK", memKB);
  u8g2.drawStr(0, 30, memLine);

  // After memory completes, show ROM BASIC message
  if (memKB >= 640)
  {
    u8g2.drawStr(0, 42, "");
    u8g2.drawStr(0, 50, "BASIC ROM OK");
  }

  u8g2.sendBuffer();
}

// ================= DOS BOOT =================
void drawBoot()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);

  u8g2.drawStr(0, 10, "IBM Cassette BASIC Version C1.00");
  u8g2.drawStr(0, 20, "Copyright IBM Corp 1981");

  if (cursor)
    u8g2.drawStr(0, 40, "Ok");

  u8g2.sendBuffer();
}

// ================= WORDSTAR =================
void updateWordStar()
{
  if (millis() - lastType > 40)
  {
    lastType = millis();
    wsChar++;

    if (wsChar > strlen(wsText[wsLine]))
    {
      wsLine++;
      wsChar = 0;

      if (wsLine >= 7)
      {
        delay(1000);
        wsLine = 0;
        wsChar = 0;
      }
    }
  }
}

void drawWordStar()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);

  // Title bar
  u8g2.drawBox(0, 0, 128, 10);
  u8g2.setDrawColor(0);
  u8g2.drawStr(2, 8, "WordStar 3.3  DEMO.TXT");
  u8g2.setDrawColor(1);

  // Menu
  u8g2.drawStr(0, 18, "^K ^S ^D ^X ^F ^Q");

  // Text
  int y = 30;
  for (int i = 0; i <= wsLine; i++)
  {
    String line = wsText[i];
    if (i == wsLine)
      line = line.substring(0, wsChar);
    u8g2.drawStr(0, y, line.c_str());
    y += 8;
  }

  // Cursor
  if (cursor)
  {
    int x = u8g2.getStrWidth(
        String(wsText[wsLine]).substring(0, wsChar).c_str());
    int ycur = 30 + wsLine * 8 - 6;
    u8g2.drawBox(x, ycur, 5, 7);
  }

  u8g2.sendBuffer();
}

// ================= MATRIX =================
void resetMatrixDrop(int i, bool randomY)
{
  drops[i].y = randomY ? random(-100, 0) : -(drops[i].length * 10);
  drops[i].speed = random(15, 40) / 10.0;
  drops[i].length = random(4, M_LEN);
  for (int k = 0; k < M_LEN; k++)
    drops[i].chars[k] = getRandomGlyph();
}

void resetMatrixAll()
{
  for (int i = 0; i < M_COLS; i++)
    resetMatrixDrop(i, true);
}

void drawMatrix()
{
  u8g2.setFont(u8g2_font_b10_t_japanese1);
  u8g2.clearBuffer();
  bool activity = false;
  for (int i = 0; i < M_COLS; i++)
  {
    drops[i].y += drops[i].speed;
    for (int j = 0; j < drops[i].length; j++)
    {
      int cy = (int)drops[i].y - (j * 10);
      if (cy > -12 && cy < 74)
      {
        if (random(0, 100) < 2)
          drops[i].chars[j] = getRandomGlyph();
        u8g2.drawGlyph(i * 8, cy, drops[i].chars[j]);
      }
    }
    if (drops[i].y - (drops[i].length * 10) > 64)
    {
      resetMatrixDrop(i, false);
      activity = true;
    }
  }
  u8g2.sendBuffer();
  if (activity)
  {
    digitalWrite(LED_PIN, HIGH);
    delayMicroseconds(200);
    digitalWrite(LED_PIN, LOW);
  }
}

// ================= PONG =================
void resetPong()
{
  ballX = 64;
  ballY = 32;
  p1Y = 26;
  p2Y = 26;
  score1 = 0;
  score2 = 0;
  dirX = 1.5;
  dirY = 1.0;
}

void resetBall()
{
  ballX = 64;
  ballY = 32;
  dirX = (random(0, 2) == 0 ? 1.5 : -1.5);
  dirY = (random(0, 2) == 0 ? 1.0 : -1.0);
  delay(200);
}

void drawPong()
{
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.clearBuffer();
  bool hit = false;
  ballX += dirX;
  ballY += dirY;
  if (ballY <= 0 || ballY >= 63)
  {
    dirY = -dirY;
    hit = true;
  }
  if (ballX <= PW + 1 && ballY + 3 >= p1Y && ballY <= p1Y + PH)
  {
    dirX = abs(dirX) + 0.1;
    ballX = PW + 2;
    hit = true;
  }
  if (ballX >= 127 - PW - 1 && ballY + 3 >= p2Y && ballY <= p2Y + PH)
  {
    dirX = -abs(dirX) - 0.1;
    ballX = 127 - PW - 2;
    hit = true;
  }
  if (ballX < 0)
  {
    score2++;
    if (score2 >= 9)
    {
      u8g2.setCursor(50, 6);
      u8g2.print(score1);
      u8g2.setCursor(74, 6);
      u8g2.print(score2);
      u8g2.sendBuffer();
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_PIN, LOW);
      resetPong();
      return;
    }
    resetBall();
    hit = true;
  }
  if (ballX > 128)
  {
    score1++;
    if (score1 >= 9)
    {
      u8g2.setCursor(50, 6);
      u8g2.print(score1);
      u8g2.setCursor(74, 6);
      u8g2.print(score2);
      u8g2.sendBuffer();
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_PIN, LOW);
      resetPong();
      return;
    }
    resetBall();
    hit = true;
  }

  if (p1Y + PH / 2 < ballY - 2)
    p1Y += 1.8;
  if (p1Y + PH / 2 > ballY + 2)
    p1Y -= 1.8;
  if (p2Y + PH / 2 < ballY - 2)
    p2Y += 1.8;
  if (p2Y + PH / 2 > ballY + 2)
    p2Y -= 1.8;
  p1Y = constrain(p1Y, 0, 64 - PH);
  p2Y = constrain(p2Y, 0, 64 - PH);

  for (int i = 0; i < 64; i += 4)
    u8g2.drawPixel(64, i);
  u8g2.setCursor(50, 6);
  u8g2.print(score1);
  u8g2.setCursor(74, 6);
  u8g2.print(score2);
  u8g2.drawBox(0, (int)p1Y, PW, PH);
  u8g2.drawBox(128 - PW, (int)p2Y, PW, PH);
  u8g2.drawBox((int)ballX, (int)ballY, 3, 3);
  u8g2.sendBuffer();
  if (hit)
  {
    digitalWrite(LED_PIN, HIGH);
    delayMicroseconds(800);
    digitalWrite(LED_PIN, LOW);
  }
}

// ================= SNAKE =================
bool isCollision(int x, int y)
{
  if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H)
    return true;
  for (int i = 0; i < snakeLen - 1; i++)
    if (snake[i].x == x && snake[i].y == y)
      return true;
  return false;
}

void spawnFood()
{
  bool valid = false;
  while (!valid)
  {
    food.x = random(0, GRID_W);
    food.y = random(0, GRID_H);
    valid = true;
    for (int i = 0; i < snakeLen; i++)
      if (snake[i].x == food.x && snake[i].y == food.y)
        valid = false;
  }
}

void resetSnake()
{
  snakeLen = 4;
  for (int i = 0; i < snakeLen; i++)
    snake[i] = {10 - i, 8};
  sDirX = 1;
  sDirY = 0;
  spawnFood();
}

void drawSnake()
{
  Point head = snake[0];
  int wishX = 0, wishY = 0;
  if (food.x > head.x)
    wishX = 1;
  else if (food.x < head.x)
    wishX = -1;
  if (food.y > head.y)
    wishY = 1;
  else if (food.y < head.y)
    wishY = -1;

  // Verbesserte KI:
  bool moved = false;
  // 1. Wunsch X probieren
  if (wishX != 0 && !isCollision(head.x + wishX, head.y))
  {
    sDirX = wishX;
    sDirY = 0;
    moved = true;
  }
  // 2. Wunsch Y probieren
  else if (wishY != 0 && !isCollision(head.x, head.y + wishY))
  {
    sDirX = 0;
    sDirY = wishY;
    moved = true;
  }

  // 3. Notfall (irgendein Weg)
  if (!moved)
  {
    int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int startIdx = random(0, 4);
    for (int i = 0; i < 4; i++)
    {
      int idx = (startIdx + i) % 4;
      if (!isCollision(head.x + dirs[idx][0], head.y + dirs[idx][1]))
      {
        sDirX = dirs[idx][0];
        sDirY = dirs[idx][1];
        moved = true;
        break;
      }
    }
  }

  // Bewegung
  for (int i = snakeLen - 1; i > 0; i--)
    snake[i] = snake[i - 1];
  snake[0].x += sDirX;
  snake[0].y += sDirY;

  // Check Crash
  if (snake[0].x < 0 || snake[0].x >= GRID_W || snake[0].y < 0 || snake[0].y >= GRID_H)
  {
    resetSnake();
    return;
  }
  for (int i = 1; i < snakeLen; i++)
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
    {
      resetSnake();
      return;
    }

  // Futter
  if (snake[0].x == food.x && snake[0].y == food.y)
  {
    if (snakeLen < 99)
    {
      snake[snakeLen] = snake[snakeLen - 1]; // Fix für Flackern!
      snakeLen++;
    }
    spawnFood();
    digitalWrite(LED_PIN, HIGH);
    delay(2);
    digitalWrite(LED_PIN, LOW);
  }

  u8g2.clearBuffer();
  u8g2.drawFrame(food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
  for (int i = 0; i < snakeLen; i++)
    u8g2.drawBox(snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE - 1, GRID_SIZE - 1);
  u8g2.setFont(u8g2_font_micro_tr);
  u8g2.setCursor(0, 5);
  u8g2.print(snakeLen);
  u8g2.sendBuffer();
  delay(60);
}

// ================= SETUP =================
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT); // TTP223

  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.setFont(u8g2_font_5x7_tr);

  randomSeed(analogRead(0));
  resetMatrixAll();
  resetPong();
  resetSnake();
}

// ================= LOOP =================
void loop()
{
  blinkCursor();

  bool changeMode = false;

  // Check for touch sensor press
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState == HIGH && lastTouchState == LOW)
  {
    changeMode = true;
    delay(50);
  }
  lastTouchState = touchState;

  // Auto change based on time
  if (millis() - lastSwitch > demoDuration)
  {
    changeMode = true;
  }

  if (changeMode)
  {
    lastSwitch = millis();
    mode = (DemoMode)((mode + 1) % 7);

    switch (mode)
    {
    case DEMO_BIOS:
      biosInit = false;
      break;
    case DEMO_WORDSTAR:
      break;
    case DEMO_SNAKE:
      resetSnake();
      break;
    case DEMO_MATRIX:
      resetMatrixAll();
      break;
    case DEMO_PONG:
      resetPong();
      break;
    }
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }

  switch (mode)
  {
  case DEMO_LOGO:
    demoDuration = 5000;
    drawIBMLogo();
    break;

  case DEMO_BIOS:
    demoDuration = 5000;
    drawBIOS();
    break;

  case DEMO_BOOT:
    demoDuration = 8000;
    drawBoot();
    break;

  case DEMO_WORDSTAR:
    demoDuration = 8000;
    updateWordStar();
    drawWordStar();
    break;

  case DEMO_MATRIX:
    demoDuration = 20000;
    drawMatrix();
    break;

  case DEMO_PONG:
    demoDuration = 20000;
    drawPong();
    break;

  case DEMO_SNAKE:
    demoDuration = 20000;
    drawSnake();
    break;
  }
}