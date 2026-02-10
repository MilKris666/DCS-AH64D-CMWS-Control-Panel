// ****************************
// *****   AH64D CMDS    ******
// *****   by MilKris    ******
// **************************** 

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#define DCSBIOS_IRQ_SERIAL
#include <DcsBios.h>

// SSD1306 128x64 I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// State
volatile bool gDirty = true;
volatile uint32_t gLastChangeMs = 0;

char gPage[5]   = "NONE"; // max 4 + '\0'
char gBit1[4]   = "";     // max 3 + '\0'
char gBit2[5]   = "";     // max 4 + '\0'
char gChaffL[2] = "";     // max 1 + '\0'
char gChaffC[4] = "";     // max 3 + '\0'
char gFlareL[2] = "";     // max 1 + '\0'
char gFlareC[4] = "";     // max 3 + '\0'

static inline void markDirty() {
  gDirty = true;
  gLastChangeMs = millis();
}

bool copyIfChanged(char* dst, size_t dstSize, const char* src) {
  if (!src) src = "";
  if (strncmp(dst, src, dstSize) == 0) return false;
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
  markDirty();
  return true;
}

// DCS-BIOS Callbacks
void onPltCmwsPageChange(char* v)        { copyIfChanged(gPage,   sizeof(gPage),   v); }
void onPltCmwsBitLine1Change(char* v)    { copyIfChanged(gBit1,   sizeof(gBit1),   v); }
void onPltCmwsBitLine2Change(char* v)    { copyIfChanged(gBit2,   sizeof(gBit2),   v); }
void onPltCmwsChaffLetterChange(char* v) { copyIfChanged(gChaffL, sizeof(gChaffL), v); }
void onPltCmwsChaffCountChange(char* v)  { copyIfChanged(gChaffC, sizeof(gChaffC), v); }
void onPltCmwsFlareLetterChange(char* v) { copyIfChanged(gFlareL, sizeof(gFlareL), v); }
void onPltCmwsFlareCountChange(char* v)  { copyIfChanged(gFlareC, sizeof(gFlareC), v); }

// DCS-BIOS Buffers

// CMWS Display Page (NONE/MAIN/TEST) MaxLen 4
DcsBios::StringBuffer<5> pltCmwsPageBuffer(0x8748, onPltCmwsPageChange);

// BIT lines: MaxLen 3 and 4
DcsBios::StringBuffer<4> pltCmwsBitLine1Buffer(0x8758, onPltCmwsBitLine1Change);
DcsBios::StringBuffer<5> pltCmwsBitLine2Buffer(0x875c, onPltCmwsBitLine2Change);

// Chaff: Letter MaxLen 1, Count MaxLen 3
DcsBios::StringBuffer<2> pltCmwsChaffLetterBuffer(0x8752, onPltCmwsChaffLetterChange);
DcsBios::StringBuffer<4> pltCmwsChaffCountBuffer(0x8754, onPltCmwsChaffCountChange);

// Flare: Letter MaxLen 1, Count MaxLen 3
DcsBios::StringBuffer<2> pltCmwsFlareLetterBuffer(0x874c, onPltCmwsFlareLetterChange);
DcsBios::StringBuffer<4> pltCmwsFlareCountBuffer(0x874e, onPltCmwsFlareCountChange);

// LED Pin Mapping (Mega)
const uint8_t PIN_CMWS_R_BRT        = 22;
const uint8_t PIN_CMWS_D_BRT        = 23;
const uint8_t PIN_CMWS_FWD_R_BRT    = 24;
const uint8_t PIN_CMWS_AFT_R_BRT    = 25;
const uint8_t PIN_CMWS_AFT_L_BRT    = 26;
const uint8_t PIN_CMWS_FWD_L_BRT    = 27;

// R light, bright (orange) - Mask 0x0100
DcsBios::LED pltCmwsRBrtL(0x873e, 0x0100, PIN_CMWS_R_BRT);

// D light, bright (orange) - Mask 0x0400
DcsBios::LED pltCmwsDBrtL(0x873e, 0x0400, PIN_CMWS_D_BRT);

// Forward right sector lights, bright (orange) - Mask 0x8000
DcsBios::LED pltCmwsFwdRightBrtL(0x873e, 0x8000, PIN_CMWS_FWD_R_BRT);

// Forward left sector lights, bright (orange) - Mask 0x1000
DcsBios::LED pltCmwsFwdLeftBrtL(0x873e, 0x1000, PIN_CMWS_FWD_L_BRT);

// Aft right sector lights, bright (orange) - Mask 0x4000
DcsBios::LED pltCmwsAftRightBrtL(0x873e, 0x4000, PIN_CMWS_AFT_R_BRT);

// Aft left sector lights, bright (orange) - Mask 0x2000
DcsBios::LED pltCmwsAftLeftBrtL(0x873e, 0x2000, PIN_CMWS_AFT_L_BRT);

// INPUT PIN MAPPING (Mega)
// 2-Pos switches
const uint8_t PIN_CMWS_ARM     = 30;
const uint8_t PIN_CMWS_BYPASS  = 31;
const uint8_t PIN_CMWS_JETT    = 32;
const uint8_t PIN_CMWS_MODE    = 33;

// 3-Pos PWR switch (OFF/ON/TEST)
const uint8_t PIN_CMWS_PW_A    = 34;
const uint8_t PIN_CMWS_PW_B    = 35;

// Rotary encoders
const uint8_t PIN_CMWS_LAMP_A  = 36;
const uint8_t PIN_CMWS_LAMP_B  = 37;

const uint8_t PIN_CMWS_VOL_A   = 38;
const uint8_t PIN_CMWS_VOL_B   = 39;

// DCS-BIOS INPUTS 
// Switches (wired to GND, use internal pullups)
DcsBios::Switch2Pos pltCmwsArm("PLT_CMWS_ARM", PIN_CMWS_ARM);
DcsBios::Switch2Pos pltCmwsBypass("PLT_CMWS_BYPASS", PIN_CMWS_BYPASS);
DcsBios::Switch2Pos pltCmwsJett("PLT_CMWS_JETT", PIN_CMWS_JETT);
DcsBios::Switch2Pos pltCmwsMode("PLT_CMWS_MODE", PIN_CMWS_MODE);

// 3-pos: OFF/ON/TEST (two pins)
DcsBios::Switch3Pos pltCmwsPw("PLT_CMWS_PW", PIN_CMWS_PW_A, PIN_CMWS_PW_B);

// Encoders (common to GND, two phases)
DcsBios::RotaryEncoder pltCmwsLamp("PLT_CMWS_LAMP", "-3200", "+3200", PIN_CMWS_LAMP_A, PIN_CMWS_LAMP_B);
DcsBios::RotaryEncoder pltCmwsVol ("PLT_CMWS_VOL",  "-3200", "+3200", PIN_CMWS_VOL_A,  PIN_CMWS_VOL_B);

// Renderig 
void drawTwoLines(const char* top, const char* bottom) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_tr);

  const int xOffset = 64;   // alles um 64 Pixel nach rechts
  const int yTop = 24;
  const int yBot = 56;

  u8g2.drawStr(xOffset, yTop, top);
  u8g2.drawStr(xOffset, yBot, bottom);
  u8g2.sendBuffer();
}

void renderFromSnapshot(const char* page,
                        const char* bit1, const char* bit2,
                        const char* chL,  const char* chC,
                        const char* flL,  const char* flC) {
  char lineTop[22];
  char lineBot[22];

  if (strncmp(page, "TEST", 4) == 0) {
    snprintf(lineTop, sizeof(lineTop), "%s", (bit1 && bit1[0]) ? bit1 : "...");
    snprintf(lineBot, sizeof(lineBot), "%s", (bit2 && bit2[0]) ? bit2 : "...");
  } else if (strncmp(page, "MAIN", 4) == 0) {
    const char* fL2 = (flL && flL[0]) ? flL : "F";
    const char* fC2 = (flC && flC[0]) ? flC : "---";
    const char* cL2 = (chL && chL[0]) ? chL : "C";
    const char* cC2 = (chC && chC[0]) ? chC : "---";

    // Top: Flare
    snprintf(lineTop, sizeof(lineTop), "%s %s", fL2, fC2);
    // Bottom: Chaff
    snprintf(lineBot, sizeof(lineBot), "%s %s", cL2, cC2);
  } else {
    snprintf(lineTop, sizeof(lineTop), "CMWS");
    snprintf(lineBot, sizeof(lineBot), "OFFLINE");
  }

  drawTwoLines(lineTop, lineBot);
}

void setup() {
  u8g2.begin();
  drawTwoLines("CMWS", "BOOT");

 
  pinMode(PIN_CMWS_R_BRT, OUTPUT);
  pinMode(PIN_CMWS_D_BRT, OUTPUT);
  pinMode(PIN_CMWS_FWD_R_BRT, OUTPUT);
  pinMode(PIN_CMWS_FWD_L_BRT, OUTPUT);
  pinMode(PIN_CMWS_AFT_R_BRT, OUTPUT);
  pinMode(PIN_CMWS_AFT_L_BRT, OUTPUT);

  digitalWrite(PIN_CMWS_R_BRT, LOW);
  digitalWrite(PIN_CMWS_D_BRT, LOW);
  digitalWrite(PIN_CMWS_FWD_R_BRT, LOW);
  digitalWrite(PIN_CMWS_FWD_L_BRT, LOW);
  digitalWrite(PIN_CMWS_AFT_R_BRT, LOW);
  digitalWrite(PIN_CMWS_AFT_L_BRT, LOW);

  DcsBios::setup();
  markDirty();
}

void loop() {
  DcsBios::loop();

  if (!gDirty) return;

 
  const uint32_t now = millis();
  if ((uint32_t)(now - gLastChangeMs) < 40) return;

  // Snapshot
  char page[5], bit1[4], bit2[5], chL[2], chC[4], flL[2], flC[4];

  noInterrupts();
  strncpy(page, gPage, sizeof(page)); page[sizeof(page)-1] = '\0';
  strncpy(bit1, gBit1, sizeof(bit1)); bit1[sizeof(bit1)-1] = '\0';
  strncpy(bit2, gBit2, sizeof(bit2)); bit2[sizeof(bit2)-1] = '\0';
  strncpy(chL,  gChaffL, sizeof(chL)); chL[sizeof(chL)-1] = '\0';
  strncpy(chC,  gChaffC, sizeof(chC)); chC[sizeof(chC)-1] = '\0';
  strncpy(flL,  gFlareL, sizeof(flL)); flL[sizeof(flL)-1] = '\0';
  strncpy(flC,  gFlareC, sizeof(flC)); flC[sizeof(flC)-1] = '\0';
  gDirty = false;
  interrupts();

  renderFromSnapshot(page, bit1, bit2, chL, chC, flL, flC);
}
