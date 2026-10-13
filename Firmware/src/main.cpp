#include <Arduino.h>
#include "soc/gpio_reg.h"

// Pin Hardware yang Sudah Ada
const int RGB[3] = {25, 26, 27};
const int buzzer = 32;
const int Buttons[2] = {12, 14};
const int DHT = 36;
const int US[2] = {16, 17};
const int servo = 19;

// Pin Kustom I2C untuk SSD1306 (Bebas Bentrok)
#define SDA_PIN 22
#define SCL_PIN 21
#define SSD1306_ADDR 0x3C

// --- Fungsi Low-Level Bit-Bang I2C ---
void i2c_delay() {
  for (volatile int i = 0; i < 10; i++);
}

void i2c_sda(bool state) {
  if (state) REG_WRITE(GPIO_OUT_W1TS_REG, (1 << SDA_PIN));
  else REG_WRITE(GPIO_OUT_W1TC_REG, (1 << SDA_PIN));
}

void i2c_scl(bool state) {
  if (state) REG_WRITE(GPIO_OUT_W1TS_REG, (1 << SCL_PIN));
  else REG_WRITE(GPIO_OUT_W1TC_REG, (1 << SCL_PIN));
}

void i2c_start() {
  i2c_sda(1); i2c_scl(1); i2c_delay();
  i2c_sda(0); i2c_delay();
  i2c_scl(0); i2c_delay();
}

void i2c_stop() {
  i2c_sda(0); i2c_scl(1); i2c_delay();
  i2c_sda(1); i2c_delay();
}

void i2c_write_byte(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    i2c_sda((data & 0x80) != 0);
    i2c_scl(1); i2c_delay();
    i2c_scl(0); i2c_delay();
    data <<= 1;
  }
  // ACK cycle
  i2c_sda(1); i2c_scl(1); i2c_delay();
  i2c_scl(0); i2c_delay();
}

void ssd1306_command(uint8_t cmd) {
  i2c_start();
  i2c_write_byte(SSD1306_ADDR << 1);
  i2c_write_byte(0x00); // Co = 0, D/C = 0
  i2c_write_byte(cmd);
  i2c_stop();
}

void ssd1306_init() {
  // Config GPIO Output
  REG_WRITE(GPIO_ENABLE_REG, REG_READ(GPIO_ENABLE_REG) | (1 << SDA_PIN) | (1 << SCL_PIN));

  // Sequence Inisialisasi Layar
  ssd1306_command(0xAE); // Display OFF
  ssd1306_command(0x20); ssd1306_command(0x00); // Horizontal Addressing Mode
  ssd1306_command(0x8D); ssd1306_command(0x14); // Enable Charge Pump
  ssd1306_command(0xAF); // Display ON
}

// Font 5x7 Font Sederhana untuk "Hello World"
const uint8_t font_H[] = {0x7F, 0x08, 0x08, 0x08, 0x7F};
const uint8_t font_e[] = {0x38, 0x54, 0x54, 0x54, 0x18};
const uint8_t font_l[] = {0x00, 0x41, 0x7F, 0x01, 0x00};
const uint8_t font_o[] = {0x38, 0x44, 0x44, 0x44, 0x38};
const uint8_t font_W[] = {0x7F, 0x20, 0x18, 0x20, 0x7F};
const uint8_t font_r[] = {0x7C, 0x08, 0x04, 0x04, 0x08};
const uint8_t font_d[] = {0x18, 0x24, 0x24, 0x1E, 0x01};

void draw_char(const uint8_t* font) {
  i2c_start();
  i2c_write_byte(SSD1306_ADDR << 1);
  i2c_write_byte(0x40); // Data Mode
  for(int i=0; i<5; i++) i2c_write_byte(font[i]);
  i2c_write_byte(0x00); // Spasi
  i2c_stop();
}

void print_hello_world() {
  // Clear RAM Layar
  ssd1306_command(0x21); ssd1306_command(0); ssd1306_command(127);
  ssd1306_command(0x22); ssd1306_command(0); ssd1306_command(7);
  
  i2c_start();
  i2c_write_byte(SSD1306_ADDR << 1);
  i2c_write_byte(0x40);
  for(int i=0; i<1024; i++) i2c_write_byte(0x00);
  i2c_stop();

  // Set Posisi Kursor ke Baris 0, Kolom 0
  ssd1306_command(0x21); ssd1306_command(0); ssd1306_command(127);
  ssd1306_command(0x22); ssd1306_command(0); ssd1306_command(0);

  // Print "Hello World"
  draw_char(font_H); draw_char(font_e); draw_char(font_l); draw_char(font_l); draw_char(font_o);
  
  // Spasi
  i2c_start(); i2c_write_byte(SSD1306_ADDR << 1); i2c_write_byte(0x40);
  i2c_write_byte(0x00); i2c_write_byte(0x00); i2c_stop();
  
  draw_char(font_W); draw_char(font_o); draw_char(font_r); draw_char(font_l); draw_char(font_d);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  ssd1306_init();
  print_hello_world();
}

void loop() {
  delay(10);
}