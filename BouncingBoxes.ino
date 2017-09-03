/**
 * ==========================================================================
 * Elegoo ILI9341 TFT LCD test/demo: bouncing boxes
 * ==========================================================================
 * 
 * Author: Michael Balzer <dexter@dexters-web.de>
 * 
 * Libraries used:
 *  - Elegoo_GFX      (Core graphics library)
 *  - Elegoo_TFTLCD   (Hardware-specific library)
 * 
 * Licenses:
 *  This is free software and information under the GNU LPGL
 *  https://www.gnu.org/licenses/lgpl.html
 */

#include <Elegoo_GFX.h>
#include <Elegoo_TFTLCD.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Our background color:
#define BACKGROUND BLACK

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

unsigned long lastms;


/**
 * class Ball: model of a bouncing "ball" (a box for now…)
 * 
 * We need to avoid redrawing the whole display to avoid flicker.
 * So we're implementing an incremental scheme of…
 *  - moving the ball
 *  - erasing the old position remains
 *  - drawing the ball at the new position
 * 
 * To make things easy for the test, the ball is just a filled box.
 * 
 * Moving includes bouncing off walls and ground, with random modifications
 * of the speed to include some nice variations. Gravity is also added.
 */
class Ball {
  float x, y, dx, dy;     // Position (x,y) and speed (dx,dy)
  int ix, iy, ox, oy;     // New (ix,iy) and old (ox,oy) integer position
  unsigned int radius;    // a random size
  unsigned int color;     // a random color

public:

  /**
   * Generate the ball:
   */
  Ball() {
    
    // init attributes:
    radius = random(6, 12);
    color = random(BLACK, WHITE);
    
    // init position:
    x = random(radius, tft.width()-radius);
    y = random(radius, tft.height()-radius);
    
    // init speed:
    dx = random(20, 45) / 10.0;
    dy = random(20, 45) / 10.0;
  }

  
  /**
   * Move the ball, bounce off walls & ground, add gravity:
   */
  void move() {
    
    // remember old ball position:
    ox = ix;
    oy = iy;
    
    // bounce:
    
    if (x+dx <= radius || x+dx >= tft.width() - radius) {
      // bounce from walls
      dx = random(20, 45) / 10.0 * ((dx > 0) ? -1 : 1);
    }
    
    if (y+dy >= tft.height() - radius) {
      // bounce from ground
      dy = random(-45, -20) / 10.0;
    }
    else {
      // add gravity
      dy += 0.05;
    }

    // move ball:
    x += dx;
    y += dy;
    
    ix = x;
    iy = y;
  }

  
  /**
   * Erase remains of old ball (ox,oy):
   */
  void erase() {
    
    // get movement distances old → new position:
    int mx = ix - ox;
    int my = iy - oy;
    
    // erase horizontal gap:
    if (mx > 0) {
      tft.fillRect(ox - radius, oy - radius, mx, radius*2, BACKGROUND);
    }
    else if (mx < 0 ) {
      tft.fillRect(ox + radius + mx, oy - radius, -mx, radius*2, BACKGROUND);
    }
  
    // erase vertical gap:
    if (my > 0) {
      tft.fillRect(ox - radius, oy - radius, radius*2, my, BACKGROUND);
    }
    else if (my < 0 ) {
      tft.fillRect(ox - radius, oy + radius + my, radius*2, -my, BACKGROUND);
    }
  }

  // Draw new ball:
  void draw() {
    tft.fillRect(ix - radius, iy - radius, radius*2, radius*2, color);
  }
  
};


/**
 * Create some balls:
 */
#define BALLCOUNT 5
Ball *balls[BALLCOUNT];


/**
 * Arduino sketch init hook:
 */
void setup() {
  
  randomSeed(analogRead(0));

  // init display:
  
  tft.reset();
  uint16_t identifier = tft.readID();
   if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
    identifier=0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    identifier=0x9341;
  }
  tft.begin(identifier);
  
  tft.fillScreen(BACKGROUND);
  tft.setRotation(3);

  
  // create balls:
  
  for (int i=0; i<BALLCOUNT; i++) {
    balls[i] = new Ball();
  }
  
  
  lastms = millis();
}


/**
 * Arduino sketch loop hook:
 */
void loop() {

  // 5 ms time steps (200 Hz):
  unsigned long ms = millis();
  if (ms - lastms <= 5) {
    return;
  }
  lastms = ms;
  
  // Move all balls:
  for (Ball *b : balls) {
    b->move();
  }

  // Erase all old balls:
  for (Ball *b : balls) {
    b->erase();
  }

  // Draw all new balls:
  for (Ball *b : balls) {
    b->draw();
  }

}
