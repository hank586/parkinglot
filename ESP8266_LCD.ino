#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
void setup() {
  Serial.begin(115200);
  Wire.begin(2,0); // set chân SDA và SCL
  lcd.clear();
  lcd.init();                       
  lcd.backlight();                  
  lcd.setCursor(0, 0);               
  lcd.print("MOI XE QUA");    
}
void loop() {

}
