/*
 * ESP32-S3 with SH1106 OLED Display (4-pin I2C)
 * Interactive Screensavers + Lively Eyes
 * Cycles between modes automatically every 5 seconds OR on touch: 
 * Bubbles -> Pipes -> 3D Maze -> Ribbons -> Lively Eyes -> 3D Stars -> Fireplace -> 8-Bit Retro -> Fish Pond
 * * Hardware Connections:
 * OLED VCC -> ESP32-S3 3.3V
 * OLED GND -> ESP32-S3 GND
 * OLED SDA -> ESP32-S3 GPIO 21
 * OLED SCL -> ESP32-S3 GPIO 22
 * TOUCH OUT -> ESP32-S3 GPIO 4
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- Hardware Definitions ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define I2C_SDA 21
#define I2C_SCL 22
#define TOUCH_PIN 4
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Shared Enums & State Management ---
enum ScreenSaverMode { BUBBLES, PIPES, MAZE, RIBBONS, EYES, STARS, FIREPLACE, RETRO_8BIT, FISH_POND, ASTEROIDS };
ScreenSaverMode currentMode = BUBBLES;

enum EyeState { NORMAL, HAPPY, SAD, BLINK }; // Moved to top for Arduino preprocessor compatibility

// Timer variables
unsigned long lastSwitchTime = 0;
const unsigned long MODE_DURATION = 10 * 1000; // 5 seconds per mode

// ==========================================
// 1. BUBBLES SCREENSAVER
// ==========================================
#define NUM_BUBBLES 8
struct Bubble {
  float x, y, vx, vy, radius;
};
Bubble bubbles[NUM_BUBBLES];

void initBubbles() {
  for (int i = 0; i < NUM_BUBBLES; i++) {
    bubbles[i].x = random(10, SCREEN_WIDTH - 10);
    bubbles[i].y = random(10, SCREEN_HEIGHT - 10);
    bubbles[i].vx = random(-20, 20) / 10.0;
    bubbles[i].vy = random(-20, 20) / 10.0;
    if (bubbles[i].vx == 0) bubbles[i].vx = 1.0;
    bubbles[i].radius = random(3, 10);
  }
}

void updateBubbles() {
  display.clearDisplay();
  for (int i = 0; i < NUM_BUBBLES; i++) {
    bubbles[i].x += bubbles[i].vx;
    bubbles[i].y += bubbles[i].vy;

    // Bounce off walls
    if (bubbles[i].x - bubbles[i].radius <= 0 || bubbles[i].x + bubbles[i].radius >= SCREEN_WIDTH) {
      bubbles[i].vx *= -1;
    }
    if (bubbles[i].y - bubbles[i].radius <= 0 || bubbles[i].y + bubbles[i].radius >= SCREEN_HEIGHT) {
      bubbles[i].vy *= -1;
    }

    display.drawCircle((int)bubbles[i].x, (int)bubbles[i].y, (int)bubbles[i].radius, SH110X_WHITE);
  }
  display.display();
  delay(20);
}

// ==========================================
// 2. PIPES SCREENSAVER
// ==========================================
int pipeX, pipeY, pipeDir;
unsigned long lastPipeClear = 0;

void initPipes() {
  display.clearDisplay();
  pipeX = SCREEN_WIDTH / 2;
  pipeY = SCREEN_HEIGHT / 2;
  pipeDir = random(0, 4);
  lastPipeClear = millis();
}

void updatePipes() {
  if (millis() - lastPipeClear > 2000) {
    display.clearDisplay();
    pipeX = random(10, SCREEN_WIDTH - 10);
    pipeY = random(10, SCREEN_HEIGHT - 10);
    lastPipeClear = millis();
  }

  int nextX = pipeX;
  int nextY = pipeY;
  int stepSize = 6;

  if (pipeDir == 0) nextY -= stepSize;
  else if (pipeDir == 1) nextX += stepSize;
  else if (pipeDir == 2) nextY += stepSize;
  else if (pipeDir == 3) nextX -= stepSize;

  if (nextX <= 0 || nextX >= SCREEN_WIDTH || nextY <= 0 || nextY >= SCREEN_HEIGHT || random(0, 5) == 0) {
    pipeDir = random(0, 4); 
  } else {
    display.drawLine(pipeX, pipeY, nextX, nextY, SH110X_WHITE);
    display.drawRect(nextX - 1, nextY - 1, 3, 3, SH110X_WHITE);
    pipeX = nextX;
    pipeY = nextY;
  }
  
  display.display();
  delay(40); 
}

// ==========================================
// 3. 3D MAZE (Wireframe Corridor)
// ==========================================
float mazeDepthOffset = 0;

void initMaze() {
  mazeDepthOffset = 0;
}

void updateMaze() {
  display.clearDisplay();
  mazeDepthOffset += 0.15; 
  if (mazeDepthOffset >= 1.0) mazeDepthOffset -= 1.0;

  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2;

  display.drawLine(0, 0, cx, cy, SH110X_WHITE);
  display.drawLine(SCREEN_WIDTH, 0, cx, cy, SH110X_WHITE);
  display.drawLine(0, SCREEN_HEIGHT, cx, cy, SH110X_WHITE);
  display.drawLine(SCREEN_WIDTH, SCREEN_HEIGHT, cx, cy, SH110X_WHITE);

  for (int i = 0; i < 5; i++) {
    float z = i + 1.0 - mazeDepthOffset;
    if (z < 0.1) z = 0.1; 
    
    int w = SCREEN_WIDTH / z;
    int h = SCREEN_HEIGHT / z;
    
    if (w < SCREEN_WIDTH * 2) {
      display.drawRect(cx - w / 2, cy - h / 2, w, h, SH110X_WHITE);
    }
  }
  
  display.display();
  delay(20);
}

// ==========================================
// 4. RIBBONS SCREENSAVER
// ==========================================
float ribbonTime = 0;

void initRibbons() {
  ribbonTime = 0;
}

void updateRibbons() {
  display.clearDisplay();
  ribbonTime += 0.15; 
  
  for (int r = 0; r < 3; r++) { 
    float phaseOffset = ribbonTime + (r * 1.5);
    float amplitude = 12.0 + sin(ribbonTime * 0.5 + r) * 10.0;
    
    int prevX = 0;
    int prevY = (SCREEN_HEIGHT / 2) + sin(phaseOffset) * amplitude;
    
    for (int x = 4; x <= SCREEN_WIDTH; x += 4) {
      int y = (SCREEN_HEIGHT / 2) + sin((x * 0.05) + phaseOffset) * amplitude;
      display.drawLine(prevX, prevY, x, y, SH110X_WHITE);
      prevX = x;
      prevY = y;
    }
  }
  
  display.display();
  delay(20);
}

// ==========================================
// 5. LIVELY EYES (SMOOTH ANIMATION ENGINE)
// ==========================================
const int baseLeftEyeX = SCREEN_WIDTH / 4;
const int baseRightEyeX = (SCREEN_WIDTH * 3) / 4;
const int baseEyeY = SCREEN_HEIGHT / 2;
const int baseEyeRadius = 15;

float currEyeX = 0, currEyeY = 0, currEyeR = 0;
float targetEyeX = 0, targetEyeY = 0, targetEyeR = 0;
EyeState displayState = NORMAL;

int eyeBehavior = 0; 
int eyeStep = 0;
unsigned long lastEyeStepTime = 0;
unsigned long stepDuration = 0;

bool isBlinking = false;
unsigned long blinkTimer = 0;

void renderFace(int offsetX, int offsetY, int radiusMod, EyeState state) {
  display.clearDisplay();
  
  int lx = baseLeftEyeX + offsetX;
  int rx = baseRightEyeX + offsetX;
  int y = baseEyeY + offsetY;
  int r = baseEyeRadius + radiusMod;

  if (state == BLINK) {
    display.fillRect(lx - r, y - 2, r * 2, 4, SH110X_WHITE);
    display.fillRect(rx - r, y - 2, r * 2, 4, SH110X_WHITE);
  } else {
    display.fillCircle(lx, y, r, SH110X_WHITE);
    display.fillCircle(rx, y, r, SH110X_WHITE);

    if (state == HAPPY) {
      display.fillRect(lx - r - 1, y, (r + 1) * 2, r + 2, SH110X_BLACK);
      display.fillRect(rx - r - 1, y, (r + 1) * 2, r + 2, SH110X_BLACK);
    } else if (state == SAD) {
      display.fillRect(lx - r - 1, y - r - 3, (r + 1) * 2, r + 3, SH110X_BLACK);
      display.fillRect(rx - r - 1, y - r - 3, (r + 1) * 2, r + 3, SH110X_BLACK);
    }
  }
  display.display();
}

void initEyes() {
  currEyeX = 0; currEyeY = 0; currEyeR = 0;
  targetEyeX = 0; targetEyeY = 0; targetEyeR = 0;
  displayState = NORMAL;
  eyeBehavior = 0;
  eyeStep = 0;
  stepDuration = 0;
  lastEyeStepTime = millis();
}

void updateEyes() {
  unsigned long now = millis();

  if (now - lastEyeStepTime >= stepDuration) {
    lastEyeStepTime = now;
    eyeStep++;

    if (eyeBehavior == 0) { 
      if (eyeStep == 1) { targetEyeX = -12; targetEyeY = 2; stepDuration = 500; displayState = NORMAL; }
      else if (eyeStep == 2) { targetEyeX = 15; targetEyeY = -8; stepDuration = 600; }
      else if (eyeStep == 3) { targetEyeX = 0; targetEyeY = 0; stepDuration = 1000; }
      else if (eyeStep == 4) { eyeBehavior = random(0, 4); eyeStep = 0; stepDuration = 0; }
    }
    else if (eyeBehavior == 1) { 
      if (eyeStep == 1) { displayState = HAPPY; targetEyeY = -12; targetEyeR = -2; stepDuration = 150; }
      else if (eyeStep == 2) { targetEyeY = 6; targetEyeR = 2; stepDuration = 150; }
      else if (eyeStep == 3) { targetEyeY = -12; targetEyeR = -2; stepDuration = 150; }
      else if (eyeStep == 4) { targetEyeY = 6; targetEyeR = 2; stepDuration = 150; }
      else if (eyeStep == 5) { targetEyeX = 0; targetEyeY = 0; targetEyeR = 0; stepDuration = 1000; }
      else if (eyeStep == 6) { eyeBehavior = random(0, 4); eyeStep = 0; stepDuration = 0; }
    }
    else if (eyeBehavior == 2) { 
      displayState = HAPPY;
      if (eyeStep < 15) {
        targetEyeX = random(-4, 5); targetEyeY = random(-4, 5); targetEyeR = 3; stepDuration = 40;
      } else if (eyeStep == 15) {
        targetEyeX = 0; targetEyeY = 0; targetEyeR = 0; displayState = NORMAL; stepDuration = 800; 
      } else if (eyeStep == 16) {
        eyeBehavior = random(0, 4); eyeStep = 0; stepDuration = 0;
      }
    }
    else if (eyeBehavior == 3) { 
      if (eyeStep == 1) { displayState = SAD; targetEyeX = 0; targetEyeY = 0; stepDuration = 400; }
      else if (eyeStep == 2) { targetEyeX = -8; targetEyeY = 12; stepDuration = 1000; }
      else if (eyeStep == 3) { targetEyeX = 0; targetEyeY = 0; displayState = NORMAL; stepDuration = 500; }
      else if (eyeStep == 4) { eyeBehavior = random(0, 4); eyeStep = 0; stepDuration = 0; }
    }
  }

  float easing = 0.2; 
  if (eyeBehavior == 2) easing = 0.8; 
  else if (eyeBehavior == 3) easing = 0.05; 
  
  currEyeX += (targetEyeX - currEyeX) * easing;
  currEyeY += (targetEyeY - currEyeY) * easing;
  currEyeR += (targetEyeR - currEyeR) * easing;

  if (isBlinking) {
    if (now - blinkTimer > 150) isBlinking = false;
  } else {
    if (random(0, 100) < 2) { 
      isBlinking = true;
      blinkTimer = now;
    }
  }
  
  EyeState actualState = isBlinking ? BLINK : displayState;
  renderFace((int)currEyeX, (int)currEyeY, (int)currEyeR, actualState);
  delay(20); 
}

// ==========================================
// 6. 3D STARFIELD (WARP SPEED)
// ==========================================
#define NUM_STARS 40
struct Star {
  float x, y, z;
};
Star stars[NUM_STARS];

void resetStar(int i) {
  stars[i].x = random(-SCREEN_WIDTH, SCREEN_WIDTH);
  stars[i].y = random(-SCREEN_HEIGHT, SCREEN_HEIGHT);
  stars[i].z = random(10, 40); 
}

void initStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    resetStar(i);
  }
}

void updateStars() {
  display.clearDisplay();
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].z -= 2.0; 
    
    if (stars[i].z <= 0.1) {
      resetStar(i);
      stars[i].z = 40; 
    }

    int sx = (stars[i].x * 16 / stars[i].z) + (SCREEN_WIDTH / 2);
    int sy = (stars[i].y * 16 / stars[i].z) + (SCREEN_HEIGHT / 2);

    if (sx < 0 || sx >= SCREEN_WIDTH || sy < 0 || sy >= SCREEN_HEIGHT) {
       resetStar(i);
       stars[i].z = 40;
       continue; 
    }

    int size = 0;
    if (stars[i].z < 10) size = 2;      
    else if (stars[i].z < 20) size = 1; 
    
    if (size == 0) {
      display.drawPixel(sx, sy, SH110X_WHITE); 
    } else {
      display.fillCircle(sx, sy, size, SH110X_WHITE);
    }
  }
  display.display();
  delay(20);
}

// ==========================================
// 7. FIREPLACE (V2 - REALISTIC FLAME SIM)
// ==========================================
#define NUM_FLAMES 25
struct Flame { 
  float x, y; 
  float w, h; 
  float speed; 
  float life; 
};
Flame flames[NUM_FLAMES];

void initFireplace() {
  for (int i = 0; i < NUM_FLAMES; i++) {
    flames[i].x = random(40, 88);
    flames[i].y = random(50, 64);
    flames[i].w = random(4, 10);
    flames[i].h = random(8, 20);
    flames[i].speed = random(5, 15) / 10.0;
    flames[i].life = random(10, 100) / 100.0;
  }
}

void updateFireplace() {
  display.clearDisplay();
  
  // Draw realistic logs
  // Log 1
  display.fillRoundRect(42, 56, 30, 6, 2, SH110X_WHITE);
  display.drawFastHLine(45, 58, 20, SH110X_BLACK);
  // Log 2
  display.fillRoundRect(62, 56, 30, 6, 2, SH110X_WHITE);
  display.drawFastHLine(65, 58, 20, SH110X_BLACK);
  // Grate
  display.drawLine(40, 62, 88, 62, SH110X_WHITE);

  for (int i = 0; i < NUM_FLAMES; i++) {
    // Move flame up
    flames[i].y -= flames[i].speed;
    flames[i].life -= 0.02;

    // Drift a bit
    flames[i].x += random(-1, 2);

    // Reset flame when it "burns out"
    if (flames[i].life <= 0 || flames[i].y < 20) {
      flames[i].x = random(45, 83);
      flames[i].y = random(54, 60);
      flames[i].w = random(6, 12);
      flames[i].h = random(10, 25);
      flames[i].speed = random(8, 20) / 10.0;
      flames[i].life = 1.0;
    }

    // Draw Flame as a tapered "tongue"
    // The flame gets thinner as it goes up
    int currentW = (int)(flames[i].w * flames[i].life);
    int currentH = (int)(flames[i].h * flames[i].life);
    
    if (currentW > 0 && currentH > 0) {
      // Draw a filled center for intensity
      display.fillRoundRect((int)flames[i].x - currentW/2, (int)flames[i].y, currentW, currentH, 2, SH110X_WHITE);
      
      // Occasionally draw a "spark" that flies higher
      if (random(0, 20) == 0) {
        display.drawPixel((int)flames[i].x + random(-5, 5), (int)flames[i].y - random(0, 10), SH110X_WHITE);
      }
    }
  }
  display.display();
  delay(30);
}

// ==========================================
// 8. 8-BIT RETRO RUNNER
// ==========================================
float runnerY = 48;
float runnerVY = 0;
bool isJumping = false;
int animFrame = 0;

struct Obstacle { float x; int w; int h; };
Obstacle obs[3];

void initRetro8Bit() {
  runnerY = 48;
  runnerVY = 0;
  isJumping = false;
  animFrame = 0;
  for (int i = 0; i < 3; i++) {
    obs[i].x = SCREEN_WIDTH + (i * 60);
    obs[i].w = random(4, 10);
    obs[i].h = random(6, 14);
  }
}

void updateRetro8Bit() {
  display.clearDisplay();
  animFrame++;

  // Draw scrolling dotted ground
  for(int i = 0; i < SCREEN_WIDTH; i += 4) {
    display.drawPixel(i - (animFrame % 4), 56, SH110X_WHITE);
  }

  // Jumping Physics
  runnerY += runnerVY;
  runnerVY += 1.0; // Gravity
  if (runnerY >= 48) {
    runnerY = 48;
    runnerVY = 0;
    isJumping = false;
  }
  
  // Autonomous look-ahead to dodge obstacles
  for (int i = 0; i < 3; i++) {
    if (obs[i].x > 10 && obs[i].x < 40 && !isJumping) {
      runnerVY = -6.5; // Jump!
      isJumping = true;
    }
  }

  // Draw Runner (8x8 character body)
  int rx = 15;
  int ry = (int)runnerY;
  display.fillRect(rx, ry, 8, 8, SH110X_WHITE);
  
  // Animate character legs
  if (isJumping) {
    display.drawLine(rx, ry + 8, rx + 2, ry + 10, SH110X_WHITE);
    display.drawLine(rx + 7, ry + 8, rx + 5, ry + 10, SH110X_WHITE);
  } else {
    if ((animFrame / 2) % 2 == 0) {
      display.drawLine(rx + 2, ry + 8, rx + 2, ry + 10, SH110X_WHITE);
      display.drawLine(rx + 5, ry + 8, rx + 5, ry + 10, SH110X_WHITE);
    } else {
      display.drawLine(rx + 1, ry + 8, rx - 1, ry + 10, SH110X_WHITE);
      display.drawLine(rx + 6, ry + 8, rx + 8, ry + 10, SH110X_WHITE);
    }
  }

  // Draw and move obstacles
  for (int i = 0; i < 3; i++) {
    obs[i].x -= 3;
    if (obs[i].x < -10) {
      obs[i].x = SCREEN_WIDTH + random(20, 60);
      obs[i].w = random(4, 10);
      obs[i].h = random(6, 14);
    }
    // Draw obstacle block sitting on the ground
    display.fillRect((int)obs[i].x, 56 - obs[i].h, obs[i].w, obs[i].h, SH110X_WHITE);
  }
  
  display.display();
  delay(20);
}

// ==========================================
// 10. ASTEROIDS SIMULATION
// ==========================================
#define NUM_ASTEROIDS 6
#define NUM_BULLETS 3

struct Asteroid {
  float x, y, vx, vy;
  int size;
  float angle;
};
Asteroid asteroids[NUM_ASTEROIDS];

struct Ship {
  float x, y, vx, vy, angle;
};
Ship ship;

struct Bullet {
  float x, y, vx, vy;
  bool active;
};
Bullet bullets[NUM_BULLETS];

void initAsteroids() {
  ship.x = SCREEN_WIDTH / 2;
  ship.y = SCREEN_HEIGHT / 2;
  ship.vx = 0; ship.vy = 0;
  ship.angle = 0;

  for (int i = 0; i < NUM_ASTEROIDS; i++) {
    asteroids[i].x = random(0, SCREEN_WIDTH);
    asteroids[i].y = random(0, SCREEN_HEIGHT);
    asteroids[i].vx = random(-15, 15) / 10.0;
    asteroids[i].vy = random(-15, 15) / 10.0;
    asteroids[i].size = random(4, 8);
    asteroids[i].angle = random(0, 360);
  }

  for (int i = 0; i < NUM_BULLETS; i++) {
    bullets[i].active = false;
  }
}

void updateAsteroids() {
  display.clearDisplay();

  // 1. Update Ship (Virtual autonomous movement)
  ship.angle += 0.02; // Slowly rotate
  ship.vx = cos(ship.angle) * 0.5;
  ship.vy = sin(ship.angle) * 0.5;
  ship.x += ship.vx;
  ship.y += ship.vy;

  // Ship wrapping
  if (ship.x < 0) ship.x = SCREEN_WIDTH;
  if (ship.x > SCREEN_WIDTH) ship.x = 0;
  if (ship.y < 0) ship.y = SCREEN_HEIGHT;
  if (ship.y > SCREEN_HEIGHT) ship.y = 0;

  // Draw Ship (Triangle)
  int sX = (int)ship.x;
  int sY = (int)ship.y;
  float noseX = sX + cos(ship.angle) * 4;
  float noseY = sY + sin(ship.angle) * 4;
  float leftX = sX + cos(ship.angle + 2.5) * 3;
  float leftY = sY + sin(ship.angle + 2.5) * 3;
  float rightX = sX + cos(ship.angle - 2.5) * 3;
  float rightY = sY + sin(ship.angle - 2.5) * 3;
  display.drawTriangle(noseX, noseY, leftX, leftY, rightX, rightY, SH110X_WHITE);

  // 2. Randomly fire bullet
  if (random(0, 50) == 0) {
    for (int i = 0; i < NUM_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = ship.x;
        bullets[i].y = ship.y;
        bullets[i].vx = cos(ship.angle) * 2.0;
        bullets[i].vy = sin(ship.angle) * 2.0;
        bullets[i].active = true;
        break;
      }
    }
  }

  // 3. Update and Draw Bullets
  for (int i = 0; i < NUM_BULLETS; i++) {
    if (!bullets[i].active) continue;
    bullets[i].x += bullets[i].vx;
    bullets[i].y += bullets[i].vy;
    display.drawPixel((int)bullets[i].x, (int)bullets[i].y, SH110X_WHITE);

    if (bullets[i].x < 0 || bullets[i].x > SCREEN_WIDTH || bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT) {
      bullets[i].active = false;
    }
  }

  // 4. Update and Draw Asteroids
  for (int i = 0; i < NUM_ASTEROIDS; i++) {
    asteroids[i].x += asteroids[i].vx;
    asteroids[i].y += asteroids[i].vy;

    // Wrapping
    if (asteroids[i].x < 0) asteroids[i].x = SCREEN_WIDTH;
    if (asteroids[i].x > SCREEN_WIDTH) asteroids[i].x = 0;
    if (asteroids[i].y < 0) asteroids[i].y = SCREEN_HEIGHT;
    if (asteroids[i].y > SCREEN_HEIGHT) asteroids[i].y = 0;

    // Draw Asteroid (Jagged Circle)
    display.drawCircle((int)asteroids[i].x, (int)asteroids[i].y, asteroids[i].size, SH110X_WHITE);
    // Add a small "crater" line for detail
    display.drawLine((int)asteroids[i].x, (int)asteroids[i].y, (int)asteroids[i].x + 2, (int)asteroids[i].y + 2, SH110X_BLACK);
  }

  display.display();
  delay(20);
}

// ==========================================
// 9. FISH POND SIMULATION
// ==========================================
#define NUM_FISH 5
struct Fish {
  float x, y;
  float angle;
  float speed;
  float tailWag;
  float wagSpeed;
  int size;
};
Fish fishes[NUM_FISH];

void initFishPond() {
  for (int i = 0; i < NUM_FISH; i++) {
    fishes[i].x = random(0, SCREEN_WIDTH);
    fishes[i].y = random(0, SCREEN_HEIGHT);
    fishes[i].angle = random(0, 360) * PI / 180.0;
    fishes[i].speed = random(5, 15) / 10.0;
    fishes[i].tailWag = random(0, 100);
    fishes[i].wagSpeed = fishes[i].speed * 0.2;
    fishes[i].size = random(3, 7);
  }
}

void updateFishPond() {
  display.clearDisplay();

  // Draw some static background Lily Pads
  display.fillCircle(20, 15, 8, SH110X_WHITE);
  display.fillTriangle(20, 15, 28, 10, 28, 20, SH110X_BLACK); // Lily pad slice
  
  display.fillCircle(110, 50, 12, SH110X_WHITE);
  display.fillTriangle(110, 50, 105, 62, 115, 62, SH110X_BLACK);

  // Update and draw fishes
  for (int i = 0; i < NUM_FISH; i++) {
    // Swimming mechanics
    fishes[i].x += cos(fishes[i].angle) * fishes[i].speed;
    fishes[i].y += sin(fishes[i].angle) * fishes[i].speed;
    fishes[i].tailWag += fishes[i].wagSpeed;

    // Gentle random steering
    if (random(0, 10) == 0) {
      fishes[i].angle += random(-20, 20) / 100.0;
    }

    // Screen wrapping (torus topology)
    if (fishes[i].x < -15) fishes[i].x = SCREEN_WIDTH + 15;
    if (fishes[i].x > SCREEN_WIDTH + 15) fishes[i].x = -15;
    if (fishes[i].y < -15) fishes[i].y = SCREEN_HEIGHT + 15;
    if (fishes[i].y > SCREEN_HEIGHT + 15) fishes[i].y = -15;

    float a = fishes[i].angle;
    float sz = fishes[i].size;
    float x = fishes[i].x;
    float y = fishes[i].y;

    // Body segment coordinates
    float headX = x + cos(a) * sz;
    float headY = y + sin(a) * sz;
    float tailBaseX = x - cos(a) * sz;
    float tailBaseY = y - sin(a) * sz;

    // Draw Fish Body (Three overlapping circles for a smooth teardrop/fish shape)
    display.fillCircle(x, y, sz / 1.5, SH110X_WHITE);                 // Mid body
    display.fillCircle(headX, headY, sz / 2, SH110X_WHITE);           // Head
    display.fillCircle(tailBaseX, tailBaseY, sz / 2.5, SH110X_WHITE); // Tail base

    // Draw Animating Tail (Triangle)
    float tailAngle = a + PI;
    float wag = sin(fishes[i].tailWag) * 0.6; // Tail wag angle offset
    float tTip1X = tailBaseX + cos(tailAngle - 0.4 + wag) * sz * 1.5;
    float tTip1Y = tailBaseY + sin(tailAngle - 0.4 + wag) * sz * 1.5;
    float tTip2X = tailBaseX + cos(tailAngle + 0.4 + wag) * sz * 1.5;
    float tTip2Y = tailBaseY + sin(tailAngle + 0.4 + wag) * sz * 1.5;
    display.fillTriangle(tailBaseX, tailBaseY, tTip1X, tTip1Y, tTip2X, tTip2Y, SH110X_WHITE);

    // Draw Animating Pectoral Fins (Lines)
    float finAngle = PI / 2.5; 
    float finWag = sin(fishes[i].tailWag + PI) * 0.3; // Fins move slightly inverse to tail
    float lFinX = x + cos(a - finAngle + finWag) * sz * 1.2;
    float lFinY = y + sin(a - finAngle + finWag) * sz * 1.2;
    float rFinX = x + cos(a + finAngle - finWag) * sz * 1.2;
    float rFinY = y + sin(a + finAngle - finWag) * sz * 1.2;
    display.drawLine(x, y, lFinX, lFinY, SH110X_WHITE);
    display.drawLine(x, y, rFinX, rFinY, SH110X_WHITE);
  }
  
  display.display();
  delay(20);
}

// ==========================================
// MAIN SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  pinMode(TOUCH_PIN, INPUT);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SH1106 allocation failed!"));
    for(;;); 
  }

  // Show intro text
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(SH110X_WHITE); 
  display.setCursor(15, 20);             
  display.println(F("Touch or Wait"));
  display.setCursor(20, 40);             
  display.println(F("Loading..."));
  display.display(); 
  delay(2000); 

  // Initialize first screensaver and set timer
  initBubbles();
  lastSwitchTime = millis();
}

void loop() {
  bool manualSwitch = false; //(digitalRead(TOUCH_PIN) == HIGH);
  bool autoSwitch = (millis() - lastSwitchTime > MODE_DURATION);

  // 1. Check for Touch input or Timer to switch modes
  if (manualSwitch || autoSwitch) {
    
    // Cycle to the next mode (10 modes total now)
    currentMode = (ScreenSaverMode)((currentMode + 1) % 10);
    
    // Initialize the newly selected mode
    switch (currentMode) {
      case BUBBLES: initBubbles(); break;
      case PIPES:   initPipes(); break;
      case MAZE:    initMaze(); break;
      case RIBBONS: initRibbons(); break;
      case EYES:    initEyes(); break;
      case STARS:   initStars(); break;
      case FIREPLACE: initFireplace(); break;
      case RETRO_8BIT: initRetro8Bit(); break;
      case FISH_POND: initFishPond(); break;
      case ASTEROIDS: initAsteroids(); break;
    }

    // Reset the switch timer
    lastSwitchTime = millis();

    // If triggered by touch, wait until the touch sensor is released 
    // so it doesn't cycle incredibly fast while held down
    if (manualSwitch) {
      while(digitalRead(TOUCH_PIN) == HIGH) {
        delay(10);
      }
      delay(50); // Small debounce delay after letting go
      // Reset timer again to ensure a full 5 seconds after releasing
      lastSwitchTime = millis(); 
    }
  }

  // 2. Update the current animation frame
  switch (currentMode) {
      case BUBBLES: updateBubbles(); break;
      case PIPES:   updatePipes(); break;
      case MAZE:    updateMaze(); break;
      case RIBBONS: updateRibbons(); break;
      case EYES:    updateEyes(); break;
      case STARS:   updateStars(); break;
      case FIREPLACE: updateFireplace(); break;
      case RETRO_8BIT: updateRetro8Bit(); break;
      case FISH_POND: updateFishPond(); break;
      case ASTEROIDS: updateAsteroids(); break;
  }
}
