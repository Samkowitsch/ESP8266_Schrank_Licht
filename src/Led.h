#ifndef _Led_H_
#define _Led_H_

    #include <Arduino.h>
    #include <FastLED.h>

    FASTLED_USING_NAMESPACE

    typedef enum{
        LED_OFF, // 0
        LED_HEX,
        LED_MODE_2,
        LED_MODE_3,
        LED_RGB
    }t_led_mode;

    #define DATA_PIN     D1
    #define NUM_LEDS    120

    #define LED_TYPE     WS2812B
    #define COLOR_ORDER GRB

    extern CRGB leds[NUM_LEDS];

    #define BRIGHTNESS  255
    #define FRAMES_PER_SECOND  100


    extern uint8_t BrightnessSetting ;
    extern uint8_t correctedBrightness ;

    extern t_led_mode Led_Mode ;
    extern uint8_t R_Value ;
    extern uint8_t G_Value ;
    extern uint8_t B_Value ;
    extern uint8_t W_Value ;

    extern uint8_t pause ;
    extern uint8_t on_off ;

    extern CRGB leds[NUM_LEDS];

    #define BRIGHTNESS  255
    #define FRAMES_PER_SECOND  100


    void setupFastLed();
    void send_Mode(t_led_mode mode);
    void setLed_Mode(t_led_mode mode);
    void sendRGBW(byte red , byte green , byte blue , byte white);
    void RGBW(byte red , byte green , byte blue , byte white);
    void setBrightness(uint8_t up_down);
    void setBrightnessAbs(uint8_t val);
    void ON_OFF();
    void ledMode();

#endif