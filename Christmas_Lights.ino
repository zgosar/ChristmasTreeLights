#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// Adafruit_NeoPixel strip = Adafruit_NeoPixel(LENGTH, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define PIN 3 // LED data pin
// Button pins.
#define PIN_DATA1 8
#define PIN_DATA2 9
#define PIN_DATA3 10
#define PIN_DATA4 11

int LENGTH = 120;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LENGTH, PIN, NEO_GRB + NEO_KHZ800);
// change to NEO_RGB if necessary.

int NO_MODES = 8;
int MAX_BRIGHTNESS = 255;
int BRIGHTNESS_STEP = 20;
int brightness = 150;
int mode = 0;
int MAX_COLOR = 5; // number of defined colors
int color_i = 0; // current color index
uint32_t color = strip.Color(255, 0, 0); // current color.
uint32_t colors[] = {
  strip.Color(255, 0, 0),
  strip.Color(0, 255, 0),
  strip.Color(0, 0, 255),
  strip.Color(255, 100, 0), // yellow
  strip.Color(255, 255, 255)};

int step_num = 0; // for keeping track the position in the current sequence.
int velocity = 10;
int MAX_VELOCITY = 10;
int wait = 0; // how many ms untill the next event in sequence.

// for tracking button presses.
bool dont_change_until_down_1 = false;
bool dont_change_until_down_2 = false;
bool dont_change_until_down_3 = false;
bool dont_change_until_down_4 = false;

String inString = ""; // serial in string.

void setup() {
  Serial.begin(9600);
  pinMode(PIN_DATA1, INPUT);
  pinMode(PIN_DATA2, INPUT);
  pinMode(PIN_DATA3, INPUT);
  pinMode(PIN_DATA4, INPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void izpis(int value){
  /* Serial print. Obsolete */
  if (value == 0){
    Serial.print('0');
  } else {
    Serial.print('1');
  }
}

int read_int(){
  /* Read the next integer in Serial. Known bug: Sometimes reads only first digits of
   *  multidigit numbers.
   */
    String LocinString = "";
    while (Serial.available() > 0){
      char inChar = Serial.read();
      Serial.print("Read char for num ");
      Serial.print(inChar);
      Serial.print("\n");
      if (isDigit(inChar)) {
        Serial.print("And it is a DIGIT!\n");
        // convert the incoming byte to a char and add it to the string:
        LocinString += (char)inChar;
        
      }
      else {
        return LocinString.toInt();
      }
    }
    return LocinString.toInt();
}

void loop() {
  bool data1 = digitalRead(PIN_DATA1);
  bool data2 = digitalRead(PIN_DATA2);
  bool data3 = digitalRead(PIN_DATA3);
  bool data4 = digitalRead(PIN_DATA4);
  if (Serial.available() > 1) {
    /* Process Serial input. IO and setting settings.*/
    char inChar = Serial.read();
    Serial.print("Read ");
    Serial.print(inChar);
    Serial.print("\n");
    if (inChar == 'M'){
      int inVal = read_int();
      if (inVal < NO_MODES){
        Serial.print("Updated mode.\n");
        mode = inVal;
        step_num = 0;
        wait = 0;
      }
    }
    if (inChar == 'C'){
      int inVal = read_int();
      Serial.print("Updating color to ");
      Serial.print(inVal);
      Serial.print("\n");
      if (inVal < MAX_COLOR){
        Serial.print("Updated color.\n");
        color_i = inVal;
        color = colors[color_i];
        wait = 0;
      }
    }
    if (inChar == 'S'){
      int inVal = read_int();
      if (0 <= inVal && inVal < MAX_VELOCITY){
        Serial.print("Updated speed.\n");
        velocity = inVal;
        wait = 0;
      }
    }
    if (inChar == 'B'){
      int inVal = read_int();
      if (0 < inVal && inVal < 255){
        Serial.print("Updated brightness.\n");
        brightness = inVal;
      }
    }
  }

  /* Process button presses.
  I am not using interrupts, so we need to know the current button states and
  the previous button states to detect presses.
  */
  if (data1 && !dont_change_until_down_1){
    dont_change_until_down_1 = true;
    mode = (mode + 1) % NO_MODES;
    step_num = 0;
    wait = 0;
  }
  if (!data1 && dont_change_until_down_1) dont_change_until_down_1 = false;

  if (data2 && !dont_change_until_down_2){
    brightness -= BRIGHTNESS_STEP;
    if (brightness < 0) brightness = MAX_BRIGHTNESS;
    dont_change_until_down_2 = true;
  }
  if (!data2 && dont_change_until_down_2) dont_change_until_down_2 = false;
  
  if (data3 && !dont_change_until_down_3){
    color_i = (color_i + 1) % MAX_COLOR;
    color = colors[color_i];
    wait = 0;
    dont_change_until_down_3 = true;
  }
  if (!data3 && dont_change_until_down_3) dont_change_until_down_3 = false;
  
  if (data4 && !dont_change_until_down_4){
    dont_change_until_down_4 = true;
    velocity--;
    wait = 0;
    if (velocity == 0) velocity = MAX_VELOCITY;
  }
  if (!data4 && dont_change_until_down_4) dont_change_until_down_4 = false;

  /* If wait == 0: execute the next step in the sequence */
  if (wait == 0){
    all_serial(data1, data2, data3, data4);
    switch (mode) {
      case 0:
        step_num = solid_color(step_num, color);
        strip.setBrightness(brightness);
        wait = 100;
        break;
      case 1:
        step_num = colorWipeStep(step_num, color);
        strip.setBrightness(brightness);
        wait = 10*velocity;
        break;
      case 2:
        step_num = fade_in_out(step_num, color);
        wait = 10*velocity;
        break;
      /*case 3:
        step_num = rainbowStep(step_num);
        wait = 10*velocity;
        break;*/
      case 3:
        step_num = rainbowCycleStep(step_num);
        wait = 10*velocity;
        strip.setBrightness(brightness);
        break;
      case 4:
        step_num = chase_every_N(step_num, 2);
        wait = 50*velocity;
        strip.setBrightness(brightness);
        break;
      case 5:
        step_num = chase_every_N(step_num, 5);
        wait = 50*velocity;
        strip.setBrightness(brightness);
        break;
      case 6:
        step_num = chase_every_N(step_num, 10);
        wait = 50*velocity;
        strip.setBrightness(brightness);
        break;    
      case 7:
        step_num = fade_in_out_every_second(step_num, color);    
        wait = 10*velocity;
        break;   
    }
  } else {
    /* If there was no new step in the sequence, delay for 1ms, reduce wait and repeat loop. */
    delay(1);
    wait--;
    }
  strip.show(); /* Update strip */
  
}

int chase_every_N(int step_num, int N){
  clear_strip();
  step_num = step_num % N;
  for (int i = step_num; i < LENGTH; i+=N){
      strip.setPixelColor(i, color);
  }
  return (step_num+1)%N;
}

int solid_color(int step_num, uint32_t color){
    for (int i = 0; i < LENGTH; i++){
      strip.setPixelColor(i, color);
  }
}

int fade_in_out(int step_num, uint32_t color){
  solid_color(1, color);
  step_num--;
  if (step_num < -brightness) step_num = brightness;
  strip.setBrightness(abs(step_num));
  return step_num;
}

uint8_t Red(uint32_t color)    {        return (color >> 16) & 0xFF;    }
uint8_t Green(uint32_t color)     {        return (color >> 8) & 0xFF;    }
uint8_t Blue(uint32_t color)     {        return color & 0xFF;    }


int fade_in_out_every_second(int step_num, uint32_t color){
  uint8_t red = Red(color);
  uint8_t green = Green(color);
  uint8_t blue = Blue(color);
  int astep_num = abs(step_num);
  uint32_t first_color = strip.Color(red/brightness*astep_num, 
    green/brightness*astep_num, 
    blue/brightness*astep_num);
  int bstep_num = brightness - astep_num;
  uint32_t second_color = strip.Color(red/brightness*bstep_num, 
    green/brightness*bstep_num, 
    blue/brightness*bstep_num);

  if (step_num < 0){
    for (int i = 0; i < LENGTH; i+=2){
        strip.setPixelColor(i, first_color);
    }
    for (int i = 1; i < LENGTH; i+=2){
        strip.setPixelColor(i, second_color);
    }
  } else {
    for (int i = 1; i < LENGTH; i+=2){
        strip.setPixelColor(i, second_color);
    }
    for (int i = 0; i < LENGTH; i+=2){
        strip.setPixelColor(i, first_color);
    }
  }
  step_num--;
  if (step_num < -brightness) step_num = brightness;
  return step_num;
}


void clear_strip(){
  for(uint16_t i=0; i < strip.numPixels(); i++) { strip.setPixelColor(i, 0);}
}

int colorWipeStep(int step_num, uint32_t c){
  clear_strip();
  if (step_num < strip.numPixels()){
    for(uint16_t i=0; i < step_num; i++) {
        strip.setPixelColor(i, c);
    }
  } else {
    for(uint16_t i=step_num-strip.numPixels(); i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
  }
   
  return (step_num + 1) % (2*strip.numPixels());
}


int rainbowStep(int step_num) {
  for(int i = 0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+step_num) & 255));
  }
  return (step_num+1);
}

/*
 * Unused examples from neopixel library.
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
*/

int rainbowCycleStep(int step_num) {
  int j = step_num;
    for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
  return (step_num+1)%256;
}

/*
 * Unused examples from neopixel library.
//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}
*/

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// Copied from neopixel example.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


void all_serial(int data1, int data2, int data3, int data4){
  /* For debuging. */
  Serial.print(data1);
  Serial.print(data2);
  Serial.print(data3);
  Serial.print(data4);
  Serial.print(' ');
  Serial.print(dont_change_until_down_1);
  Serial.print(dont_change_until_down_2);
  Serial.print(dont_change_until_down_3);
  Serial.print(dont_change_until_down_4);
  Serial.print(' ');
  Serial.print(mode);
  Serial.print(' ');
  Serial.print(brightness);
  Serial.print(' ');
  Serial.print(color_i);
  Serial.print(' ');
  Serial.print(velocity);
  Serial.print(" step_num: ");
  Serial.print(step_num);
  Serial.print(' ');
  Serial.println("");
}


/*
 * Obsolete.
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void solid_red(int wait){
    for (int i = 0; i < LENGTH; i++){
      strip.setPixelColor(i, brightness, 0, 0);
  }
  strip.show();
  //delay(wait);
}

void red_chase(int wait){
    for (int i = 0; i < LENGTH; i++){
      if (i > 0) strip.setPixelColor(i-1, 0, 0, 0);
      else strip.setPixelColor(LENGTH-1, 0, 0, 0);
      strip.setPixelColor(i, brightness, 0, 0);
      strip.show();
      delay(wait);
  }
}




*/
