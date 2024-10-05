#include <Arduino.h>

#include <IRremote.hpp>
#include <IRProtocol.h>
#include <IRFeedbackLED.hpp>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
#define OLED_TEXT_SIZE 2
#define DELAY_IDLE 150

#if !defined(OLED_SDA) || !defined(OLED_SCL)
    #if defined(ARDUINO_ARCH_RP2040)
        #define OLED_SDA PIN_WIRE0_SDA
        #define OLED_SCL PIN_WIRE0_SCL
    #elif defined(ARDUINO_ARCH_ESP32)
        #define OLED_SDA SDA
        #define OLED_SCL SCL
    #endif
#endif

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
#ifdef RGB_PIN
    Adafruit_NeoPixel pixels(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);
#else
    #warning "Compiling with no RGB Led Indicator support"
#endif

bool screenAvailable = false;
bool changedOnce = false;
int lastUpdate = 0;

void initDisplayParams()
{
    display.clearDisplay();
    display.setTextSize(OLED_TEXT_SIZE);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.cp437(true);
}

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
    #ifdef RGB_PIN
        pixels.clear();
        pixels.fill(pixels.Color(r, g, b));
        pixels.setBrightness(10);
        pixels.show();
    #endif
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Setting SDA/SCL to 0/1");

    #ifdef ARDUINO_ARCH_RP2040
        Wire.setSCL(OLED_SCL);
        Wire.setSDA(OLED_SDA);
    #else
        Wire.setPins(OLED_SDA, OLED_SCL);
    #endif

    #ifdef RGB_PIN
        Serial.println("Initializing RGB LED");
        pixels.begin();
        setColor(168, 50, 153);
    #endif

    if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        screenAvailable = true;
        Serial.println("OLED Screen available");
        initDisplayParams();
        #ifdef SPANISH
            display.println("Bienvenido");
            display.println("");
            display.println("Presione");
            display.println("un boton");
        #else
            display.println("Welcome");
            display.println("");
            display.println("press");
            display.println("a button");
        #endif
        display.display();
    }
    else
    {
        Serial.println("OLED Screen NOT available");
    }

    Serial.println("Initializing IR");

    setLEDFeedback(13, LED_FEEDBACK_ENABLED_FOR_RECEIVE);
    IrReceiver.begin(29, true, 25);
}

void loop()
{
    if (!screenAvailable)
    {
        return;
    }

    if (IrReceiver.decode())
    {
        initDisplayParams();
        display.print("P:");
        display.println(IrReceiver.decodedIRData.protocol);
        display.print("C:");
        display.println(IrReceiver.decodedIRData.command);
        display.print("W:");
        display.println(millis());
        display.print("R:");
        display.println(IrReceiver.decodedIRData.decodedRawData);
        display.display();

        setColor(255, 165, 0);

        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.resume();

        lastUpdate = millis();
        changedOnce = true;
    }
    else if (changedOnce && lastUpdate + DELAY_IDLE < millis())
    {
        #ifdef RGB_PIN
            pixels.setBrightness(0);
            pixels.show();
        #endif
    }
}
