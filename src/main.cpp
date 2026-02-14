#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// ==========================================
// HARDWARE KONFIGURATION
// ==========================================
// Display: SH1106 I2C (SCL an Pin 22, SDA an Pin 21)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 22, 21);

const int LED_PIN = 23;  // Rote HDD LED (über Widerstand!)
const int TOUCH_PIN = 4; // TTP223 Touch Sensor (Signal)

const unsigned long SWITCH_INTERVAL = 20000; // Auto-Wechsel alle 20s

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

// --- GLOBALE VARIABLEN ---
int currentMode = 0; // 0=Hacker, 1=Matrix, 2=Pong, 3=Snake
unsigned long lastSwitchTime = 0;
int lastTouchState = LOW;

// ==========================================
// MODULE 0: IBM LOGO
// ==========================================

void resetIBMLogo()
{
  u8g2.clearBuffer();
}

void runIBMLogo()
{
  u8g2.clearBuffer();
  u8g2.drawXBMP(28, 15, 72, 34, ibm_logo_bits);
  u8g2.sendBuffer();
}

// ==========================================
// MODUL 1: HACKER TERMINAL
// ==========================================
const int TERM_LINES = 6;
const int TERM_HEIGHT = 10;
String termBuffer[TERM_LINES];
const char *phrases[] = {
    "CONNECTING...", "UPLOADING VIRUS", "BYPASSING FIREWALL",
    "ACCESS GRANTED", "DECRYPTING KEY...", "DOWNLOADING DATA",
    "SYSTEM OVERRIDE", "ROOT ACCESS: YES", "SCANNING PORTS...",
    "INITIALIZING...", "BUFFER OVERFLOW", "TRACING IP...",
    "COMPILING...", "UPDATING KERNEL"};

void resetHacker()
{
  for (int i = 0; i < TERM_LINES; i++)
    termBuffer[i] = "";
}

String getHexDump()
{
  String hexLine = "";
  for (int i = 0; i < 3; i++)
  {
    int r = random(0, 255);
    if (r < 16)
      hexLine += "0";
    hexLine += String(r, HEX) + " ";
  }
  hexLine.toUpperCase();
  return hexLine;
}

void runHacker()
{
  u8g2.setFont(u8g2_font_micro_tr);
  String nextLine;
  if (random(0, 10) < 4)
    nextLine = phrases[random(0, 14)];
  else
    nextLine = "MEM: " + getHexDump();
  for (int i = 0; i < TERM_LINES - 1; i++)
    termBuffer[i] = termBuffer[i + 1];
  termBuffer[TERM_LINES - 1] = "> " + nextLine;
  u8g2.clearBuffer();
  for (int i = 0; i < TERM_LINES; i++)
  {
    u8g2.setCursor(0, (i + 1) * TERM_HEIGHT - 1);
    u8g2.print(termBuffer[i]);
  }
  if (millis() % 500 < 250)
  {
    u8g2.setCursor(120, 63);
    u8g2.print("_");
  }
  u8g2.sendBuffer();
  digitalWrite(LED_PIN, HIGH);
  delay(random(20, 50));
  digitalWrite(LED_PIN, LOW);
  delay(random(50, 200));
}

// ==========================================
// MODUL 2: MATRIX RAIN
// ==========================================
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

void runMatrix()
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

// ==========================================
// MODUL 3: PONG (Mit 9-Punkte-Reset)
// ==========================================
float ballX, ballY, dirX, dirY, p1Y, p2Y;
int score1, score2;
const int PW = 3, PH = 12;

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

void runPong()
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

// ==========================================
// MODUL 4: AUTO-SNAKE (Robust & Fix)
// ==========================================
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

void runSnake()
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

// ==========================================
// MAIN SETUP (MIT IBM BOOT LOGO)
// ==========================================
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT); // TTP223

  u8g2.setBusClock(400000);
  u8g2.begin();

  // --- BOOT SEQUENZ ANFANG ---

  // 1. IBM Logo zeichnen (zentriert)
  // Das Logo ist 72px breit und 34px hoch.
  // X = (128-72)/2 = 28
  // Y = (64-34)/2 = 15
  // u8g2.clearBuffer();
  // u8g2.drawXBMP(28, 15, 72, 34, ibm_logo_bits);
  // u8g2.sendBuffer();

  runIBMLogo();

  // 2. LED "POST" Check (zweimal kurz blinken)
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);

  // 3. Warten (Boot-Dauer)
  delay(5000);

  // --- BOOT SEQUENZ ENDE ---

  randomSeed(analogRead(0));
  resetHacker();
  resetMatrixAll();
  resetPong();
  resetSnake();
  lastSwitchTime = millis();
}

// ==========================================
// LOOP (MIT TOUCH & AUTO-SWITCH)
// ==========================================
void loop()
{
  bool switchMode = false;

  // 1. TOUCH SENSOR CHECK
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState == HIGH && lastTouchState == LOW)
  {
    // Flanke erkannt (Finger hat gerade berührt)
    switchMode = true;
    delay(50); // Kleines Entprellen
  }
  lastTouchState = touchState;

  // 2. TIMER CHECK
  if (millis() - lastSwitchTime > SWITCH_INTERVAL)
  {
    switchMode = true;
  }

  // 3. MODUS WECHSELN?
  if (switchMode)
  {
    lastSwitchTime = millis(); // Timer zurücksetzen
    currentMode++;
    if (currentMode > 4)
      currentMode = 0;

    // Reset Logic
    if (currentMode == 0)
      resetHacker();
    if (currentMode == 1)
      resetMatrixAll();
    if (currentMode == 2)
      resetPong();
    if (currentMode == 3)
      resetSnake();
    if (currentMode == 4)
      resetIBMLogo();

    // Kurzes LED Feedback beim Umschalten
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }

  // Programm ausführen
  switch (currentMode)
  {
  case 0:
    runHacker();
    break;
  case 1:
    runMatrix();
    break;
  case 2:
    runPong();
    break;
  case 3:
    runSnake();
    break;
  case 4:
    runIBMLogo();
    break;
  }
}
