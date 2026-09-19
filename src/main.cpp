#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>

// ============================================================
// ForgeUI MicroRacer
// ESP32-S3 + ST7789 284x76 + Analog Joystick
// ============================================================

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frame = TFT_eSprite(&tft);

constexpr int W = 284;
constexpr int H = 76;

// Physically proven joystick wiring
constexpr int JOY_X  = 6;
constexpr int JOY_Y  = 5;
constexpr int JOY_SW = 4;

// Colours
constexpr uint16_t COL_BG       = TFT_BLACK;
constexpr uint16_t COL_WHITE    = TFT_WHITE;
constexpr uint16_t COL_CYAN     = TFT_CYAN;
constexpr uint16_t COL_GREEN    = TFT_GREEN;
constexpr uint16_t COL_YELLOW   = TFT_YELLOW;
constexpr uint16_t COL_RED      = TFT_RED;
constexpr uint16_t COL_BLUE     = TFT_BLUE;
constexpr uint16_t COL_GREY     = 0x8410;
constexpr uint16_t COL_DKGREY   = 0x3186;
constexpr uint16_t COL_ROAD     = 0x2104;

// ============================================================
// Game state
// ============================================================

enum GameState
{
    TITLE,
    PLAYING,
    CRASH,
    GAME_OVER
};

GameState state = TITLE;

int joyCentreX = 2048;
int joyCentreY = 2048;

float playerX = 38.0f;
float playerY = 38.0f;

float roadOffset = 0.0f;
float speedValue = 1.8f;

uint32_t score = 0;
uint32_t highScore = 0;

float boost = 100.0f;
bool boosting = false;

unsigned long stateStart = 0;
unsigned long lastFrame = 0;

// ============================================================
// Traffic
// ============================================================

struct TrafficCar
{
    float x;
    float y;
    float speed;
    uint16_t colour;
    bool active;
};

constexpr int TRAFFIC_COUNT = 5;
TrafficCar traffic[TRAFFIC_COUNT];

// ============================================================
// Crash particles
// ============================================================

struct Particle
{
    float x;
    float y;
    float vx;
    float vy;
    int life;
    uint16_t colour;
};

constexpr int PARTICLE_COUNT = 18;
Particle particles[PARTICLE_COUNT];

// ============================================================
// Helpers
// ============================================================

bool buttonPressed()
{
    return digitalRead(JOY_SW) == LOW;
}

void pushFrame()
{
    frame.pushSprite(0, 0);
}

void centreText(const String &text, int y, int font, uint16_t colour)
{
    frame.setTextDatum(MC_DATUM);
    frame.setTextFont(font);
    frame.setTextColor(colour, COL_BG);
    frame.drawString(text, W / 2, y);
}

float joystickAxis(int raw, int centre)
{
    constexpr int deadZone = 180;

    int delta = raw - centre;

    if (abs(delta) < deadZone)
        return 0.0f;

    float value;

    if (delta > 0)
        value = (float)(delta - deadZone) /
                (4095 - centre - deadZone);
    else
        value = (float)(delta + deadZone) /
                (centre - deadZone);

    return constrain(value, -1.0f, 1.0f);
}

// ============================================================
// Joystick calibration
// ============================================================

void calibrateJoystick()
{
    long totalX = 0;
    long totalY = 0;

    constexpr int samples = 64;

    frame.fillSprite(COL_BG);
    centreText("FORGEUI", 25, 4, COL_CYAN);
    centreText("CALIBRATING JOYSTICK", 52, 1, COL_WHITE);
    pushFrame();

    for (int i = 0; i < samples; i++)
    {
        totalX += analogRead(JOY_X);
        totalY += analogRead(JOY_Y);
        delay(5);
    }

    joyCentreX = totalX / samples;
    joyCentreY = totalY / samples;

    Serial.printf(
        "Joystick centre X=%d Y=%d\n",
        joyCentreX,
        joyCentreY
    );
}

// ============================================================
// Traffic
// ============================================================

void spawnTraffic(TrafficCar &car, float offset)
{
    car.x = W + offset;

    // Road usable vertical area
    car.y = random(24, 59);

    car.speed = random(5, 16) * 0.05f;

    const uint16_t colours[] =
    {
        COL_RED,
        COL_YELLOW,
        COL_CYAN,
        COL_WHITE,
        COL_BLUE
    };

    car.colour = colours[random(0, 5)];
    car.active = true;
}

void resetTraffic()
{
    for (int i = 0; i < TRAFFIC_COUNT; i++)
        spawnTraffic(traffic[i], 55.0f + i * 65.0f);
}

// ============================================================
// Drawing
// ============================================================

void drawRoad()
{
    // Grass/background
    frame.fillSprite(COL_BG);

    // Road
    frame.fillRect(0, 17, W, 50, COL_ROAD);

    // Road edges
    frame.drawFastHLine(0, 17, W, COL_CYAN);
    frame.drawFastHLine(0, 66, W, COL_BLUE);

    // Scrolling lane markers
    constexpr int markerWidth = 22;
    constexpr int gap = 18;
    constexpr int spacing = markerWidth + gap;

    int offset = ((int)roadOffset) % spacing;

    for (int x = -spacing; x < W + spacing; x += spacing)
    {
        int px = x - offset;

        frame.fillRect(
            px,
            41,
            markerWidth,
            2,
            COL_GREY
        );
    }
}

void drawPlayer()
{
    int x = (int)playerX;
    int y = (int)playerY;

    // Shadow
    frame.fillRect(x - 7, y - 4, 16, 9, COL_DKGREY);

    // Car body
    frame.fillRoundRect(
        x - 7,
        y - 5,
        14,
        10,
        2,
        COL_CYAN
    );

    // Nose
    frame.fillRect(
        x + 5,
        y - 3,
        4,
        6,
        COL_WHITE
    );

    // Cockpit
    frame.fillRect(
        x - 2,
        y - 3,
        4,
        6,
        COL_BLUE
    );

    // Wheels
    frame.fillRect(x - 5, y - 7, 4, 2, COL_WHITE);
    frame.fillRect(x - 5, y + 5, 4, 2, COL_WHITE);

    if (boosting)
    {
        frame.drawFastHLine(x - 12, y - 2, 5, COL_YELLOW);
        frame.drawFastHLine(x - 15, y, 8, COL_RED);
        frame.drawFastHLine(x - 12, y + 2, 5, COL_YELLOW);
    }
}

void drawTrafficCar(const TrafficCar &car)
{
    int x = (int)car.x;
    int y = (int)car.y;

    frame.fillRoundRect(
        x - 7,
        y - 5,
        14,
        10,
        2,
        car.colour
    );

    frame.fillRect(
        x - 5,
        y - 3,
        4,
        6,
        COL_DKGREY
    );

    frame.fillRect(x + 4, y - 6, 3, 2, COL_WHITE);
    frame.fillRect(x + 4, y + 4, 3, 2, COL_WHITE);
}

void drawHUD()
{
    frame.setTextDatum(TL_DATUM);
    frame.setTextFont(1);

    frame.setTextColor(COL_CYAN, COL_BG);
    frame.drawString("FORGEUI", 3, 3);

    char scoreText[24];
    snprintf(scoreText, sizeof(scoreText), "S:%06lu", score);

    frame.setTextColor(COL_WHITE, COL_BG);
    frame.drawString(scoreText, 65, 3);

    int shownSpeed = (int)(speedValue * 55.0f);

    char speedText[20];
    snprintf(speedText, sizeof(speedText), "%03d", shownSpeed);

    frame.setTextColor(COL_YELLOW, COL_BG);
    frame.drawString(speedText, 157, 3);

    frame.setTextColor(COL_WHITE, COL_BG);
    frame.drawString("BOOST", 202, 3);

    frame.drawRect(240, 4, 40, 7, COL_GREY);

    int boostWidth = map((int)boost, 0, 100, 0, 36);

    uint16_t boostColour =
        boost > 25 ? COL_GREEN : COL_RED;

    frame.fillRect(
        242,
        6,
        boostWidth,
        3,
        boostColour
    );
}

// ============================================================
// Collision
// ============================================================

bool collisionWith(const TrafficCar &car)
{
    float dx = fabs(playerX - car.x);
    float dy = fabs(playerY - car.y);

    return dx < 12.0f && dy < 9.0f;
}

// ============================================================
// Crash
// ============================================================

void startCrash()
{
    state = CRASH;
    stateStart = millis();

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].x = playerX;
        particles[i].y = playerY;

        particles[i].vx =
            random(-25, 26) * 0.10f;

        particles[i].vy =
            random(-20, 21) * 0.10f;

        particles[i].life =
            random(12, 30);

        particles[i].colour =
            random(0, 2)
            ? COL_YELLOW
            : COL_RED;
    }

    if (score > highScore)
        highScore = score;
}

void updateCrash()
{
    drawRoad();

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        Particle &p = particles[i];

        if (p.life <= 0)
            continue;

        p.x += p.vx;
        p.y += p.vy;

        p.vx *= 0.96f;
        p.vy *= 0.96f;

        p.life--;

        frame.fillRect(
            (int)p.x,
            (int)p.y,
            2,
            2,
            p.colour
        );
    }

    centreText("CRASH!", 38, 4, COL_RED);

    pushFrame();

    if (millis() - stateStart > 1400)
    {
        state = GAME_OVER;
        stateStart = millis();
    }
}

// ============================================================
// Title
// ============================================================

void drawTitle()
{
    frame.fillSprite(COL_BG);

    frame.drawFastHLine(22, 12, 240, COL_CYAN);
    frame.drawFastHLine(22, 63, 240, COL_BLUE);

    centreText("FORGEUI", 27, 4, COL_CYAN);
    centreText("MICRO RACER", 47, 2, COL_WHITE);

    bool flash = ((millis() / 450) % 2) == 0;

    if (flash)
        centreText("PRESS STICK TO START", 61, 1, COL_GREEN);

    pushFrame();
}

// ============================================================
// Game over
// ============================================================

void drawGameOver()
{
    frame.fillSprite(COL_BG);

    centreText("GAME OVER", 18, 4, COL_RED);

    char scoreLine[40];
    snprintf(
        scoreLine,
        sizeof(scoreLine),
        "SCORE %lu   BEST %lu",
        score,
        highScore
    );

    centreText(scoreLine, 43, 2, COL_WHITE);

    bool flash = ((millis() / 450) % 2) == 0;

    if (flash)
        centreText("PRESS TO RACE AGAIN", 64, 1, COL_GREEN);

    pushFrame();
}

// ============================================================
// New game
// ============================================================

void startGame()
{
    playerX = 38.0f;
    playerY = 41.0f;

    speedValue = 1.8f;
    roadOffset = 0.0f;

    score = 0;
    boost = 100.0f;

    resetTraffic();

    state = PLAYING;
    stateStart = millis();
}

// ============================================================
// Gameplay
// ============================================================

void updateGame()
{
    int rawX = analogRead(JOY_X);
    int rawY = analogRead(JOY_Y);

    float inputX =
        joystickAxis(rawX, joyCentreX);

    float inputY =
        joystickAxis(rawY, joyCentreY);

    // Depending on physical joystick orientation,
    // one or both signs may later be inverted.
    playerX += inputX * 2.4f;
    playerY += inputY * 2.0f;

    playerX = constrain(playerX, 18.0f, 92.0f);
    playerY = constrain(playerY, 24.0f, 59.0f);

    boosting = buttonPressed() && boost > 1.0f;

    float currentSpeed = speedValue;

    if (boosting)
    {
        currentSpeed *= 1.65f;
        boost -= 1.15f;
    }
    else
    {
        boost += 0.22f;
    }

    boost = constrain(boost, 0.0f, 100.0f);

    roadOffset += currentSpeed * 4.0f;

    // Difficulty gradually increases.
    speedValue += 0.0008f;

    if (speedValue > 4.7f)
        speedValue = 4.7f;

    drawRoad();

    for (int i = 0; i < TRAFFIC_COUNT; i++)
    {
        TrafficCar &car = traffic[i];

        car.x -=
            currentSpeed * (2.0f + car.speed);

        if (car.x < -15)
        {
            spawnTraffic(
                car,
                random(20, 110)
            );

            score += 100;
        }

        drawTrafficCar(car);

        if (collisionWith(car))
        {
            startCrash();
            return;
        }
    }

    drawPlayer();
    drawHUD();

    pushFrame();

    score++;
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println("==============================");
    Serial.println("FORGEUI MICRO RACER");
    Serial.println("ESP32-S3 + ST7789 284x76");
    Serial.println("JOY X=6 Y=5 SW=4");
    Serial.println("==============================");

    pinMode(JOY_SW, INPUT_PULLUP);

    analogReadResolution(12);

    // Physically proven display configuration.
    tft.init();
    tft.invertDisplay(false);
    tft.setRotation(1);

    frame.setColorDepth(16);

    if (frame.createSprite(W, H) == nullptr)
    {
        Serial.println("ERROR: framebuffer allocation failed");

        while (true)
            delay(1000);
    }

    randomSeed(
        analogRead(JOY_X) ^
        micros()
    );

    calibrateJoystick();

    state = TITLE;
    stateStart = millis();

    // Prevent calibration/start button overlap.
    while (buttonPressed())
        delay(10);
}

// ============================================================
// Main loop
// ============================================================

void loop()
{
    // ~30 FPS
    if (millis() - lastFrame < 33)
        return;

    lastFrame = millis();

    static bool previousButton = false;

    bool currentButton = buttonPressed();
    bool buttonEdge =
        currentButton && !previousButton;

    previousButton = currentButton;

    switch (state)
    {
        case TITLE:
            drawTitle();

            if (buttonEdge)
                startGame();

            break;

        case PLAYING:
            updateGame();
            break;

        case CRASH:
            updateCrash();
            break;

        case GAME_OVER:
            drawGameOver();

            if (buttonEdge)
                startGame();

            break;
    }
}