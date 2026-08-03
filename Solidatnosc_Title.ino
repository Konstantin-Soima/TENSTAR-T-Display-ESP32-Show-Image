#include <TFT_eSPI.h>
#include "solidarnosc_logo.h"

#define TFT_BL_PIN    4
#define BL_PWM_FREQ   5000
#define BL_PWM_RES    8

#define BTN_LEFT_PIN  0  // GPIO0 / BOOT, has internal pull-up
#define BTN_RIGHT_PIN 35 // input-only pin, relies on board's own pull-up
#define DEBOUNCE_MS   200

TFT_eSPI tft = TFT_eSPI();

int brightness = 25; // percent
bool btnLeftLast = HIGH;
bool btnRightLast = HIGH;
unsigned long lastPressMs = 0;

void applyBrightness() {
  brightness = constrain(brightness, 0, 100);
  // PWM duty vs perceived LED brightness is non-linear (eye is far more
  // sensitive at the low end), so map the linear 0-100% control through a
  // gamma curve instead of writing the percentage straight to the duty cycle.
  const float gamma = 2.2f;
  int maxDuty = (1 << BL_PWM_RES) - 1;
  int duty = round(pow(brightness / 100.0f, gamma) * maxDuty);
  ledcWrite(TFT_BL_PIN, duty);
}

void setup() {
  tft.init();
  tft.setRotation(3); // landscape, board turned 90° counter-clockwise from native portrait
  tft.fillScreen(TFT_WHITE);

  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT);

  ledcAttach(TFT_BL_PIN, BL_PWM_FREQ, BL_PWM_RES);
  applyBrightness();

  tft.setSwapBytes(true); // logo bytes are little-endian (from ffmpeg), TFT_eSPI expects big-endian by default
  tft.pushImage(0, 0, SOLIDARNOSC_LOGO_WIDTH, SOLIDARNOSC_LOGO_HEIGHT, solidarnosc_logo);
}

void loop() {
  bool btnLeft = digitalRead(BTN_LEFT_PIN);
  bool btnRight = digitalRead(BTN_RIGHT_PIN);
  unsigned long now = millis();

  if (btnLeft == LOW && btnLeftLast == HIGH && now - lastPressMs > DEBOUNCE_MS) {
    brightness += 10;
    applyBrightness();
    lastPressMs = now;
  }
  if (btnRight == LOW && btnRightLast == HIGH && now - lastPressMs > DEBOUNCE_MS) {
    brightness -= 10;
    applyBrightness();
    lastPressMs = now;
  }

  btnLeftLast = btnLeft;
  btnRightLast = btnRight;
}
