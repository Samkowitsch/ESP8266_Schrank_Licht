#include <Led.h>
#include <Web.h>

uint32_t prev_millis = 0;

uint8_t BrightnessSetting  = 255;
uint8_t correctedBrightness ;

t_led_mode Led_Mode = LED_OFF;
uint8_t prev_Led_Mode = 0;
uint8_t R_Value = 0;
uint8_t G_Value = 0;
uint8_t B_Value = 0;
uint8_t W_Value = 0;

uint8_t R_Value_Old = 0;
uint8_t G_Value_Old = 0;
uint8_t B_Value_Old = 0;
uint8_t W_Value_Old = 0;

uint8_t pause = 0;
uint8_t on_off = 1;

uint8_t startHue = 0;

uint16_t speed = 20;

uint32_t millis_old = 0;

CRGB leds[NUM_LEDS];

void setupFastLed(){
    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void send_Mode(t_led_mode mode){

}

void setLed_Mode(t_led_mode mode){

  if(Led_Mode != LED_OFF){
    prev_Led_Mode = Led_Mode;
  }
  Led_Mode = mode; 
}

void sendRGBW(byte red , byte green , byte blue , byte white){
    uint32_t hex_val = red << 24 | green << 16 | blue << 8 | white;
}

void RGBW(byte red , byte green , byte blue , byte white){
  R_Value = red;
  G_Value = green;
  B_Value = blue;
  W_Value = white;
  fill_solid(leds , NUM_LEDS , CRGB(red , green , blue));

  if(R_Value != R_Value_Old || G_Value != G_Value_Old || B_Value != B_Value_Old || W_Value != W_Value_Old){
    // char buffer[64];
    // sprintf(buffer , "{ \"Red\":%d, \"Green\":%d, \"Blue\":%d, \"White\":%d}", R_Value , G_Value , B_Value , W_Value);
    // client.publish("Kummer/Licht/Bett_Links/RGBW" ,  buffer  ,true);

    // uint32_t hex_val = R_Value << 24 | G_Value << 16 | B_Value << 8 | W_Value;
    // sprintf(buffer , "%X" , hex_val);
    // client.publish("Kummer/Licht/Bett_Links/RGBW_HEX", buffer ,true );
    R_Value_Old = R_Value;
    G_Value_Old = G_Value;
    B_Value_Old = B_Value;
    W_Value_Old = W_Value;
  }
}

void setBrightness(uint8_t up_down){
  if(up_down){
    if(BrightnessSetting < 255)
      BrightnessSetting += 32;
  }else{
    if(BrightnessSetting > 31)
      BrightnessSetting -= 32;
  }

  correctedBrightness = dim8_video(BrightnessSetting);
  FastLED.setBrightness(correctedBrightness);
  FastLED.show();
}

void setBrightnessAbs(uint8_t val){
  correctedBrightness = dim8_video(val);
  FastLED.setBrightness(correctedBrightness);
  FastLED.show();
}

void ON_OFF(){
  if(on_off){
    //send_Mode(prev_Led_Mode);
  } else {
    //send_Mode(0);
  }
}


void ledMode(){

  switch (Led_Mode){
    case LED_OFF:
      FastLED.clear();
      break;
    case LED_HEX:
      RGBW(R_Value,G_Value,B_Value,W_Value);
      break;
    case LED_MODE_2:
      break;
    case LED_MODE_3:
      break;
    case LED_RGB:
      fill_rainbow(leds , NUM_LEDS , startHue , 1);
      break; 
    default:
      break;
  }
  FastLED.show();

  if(millis() > millis_old + speed){
    millis_old = millis();
    startHue++;
  }
}