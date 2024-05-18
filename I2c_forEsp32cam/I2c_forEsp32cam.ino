/*
檔案名：I2C_forESP32cam
更新時間：2024/05/18
功能簡介：
  目的：建立一個esp32 cam 的 I2C 主機, 並且傳訊息給地址0x39的從機, 預設從機為arduino uno, 傳輸訊息為以下步驟：
      1. esp32 cam 傳輸 "hello \n 128" 給 0x39位置的arduino uno
      2. arduino uno 接收指令, 並且逐個byte印製資料, 印製到 \n 位元資料時進行換行
      3. esp32 cam 隨後向 0x39位置的arduino uno 請求一筆 4byte 的資料, 前兩個byte組合成"hi", 後兩個byte組合成整數 216
      4. arduino uno 在收到請求後向主機 esp32 cam 發送 "h" "i" "216"

  接線：esp32 cam             arduino
      pin 12 (sda)    ->      pin A4 (sda)
      pin 13 (scl)    ->      pin A5 (scl)
      pin GND         ->      pin GND     

  備註：
    1. 這個程式碼要上傳到esp32 cam, 並且搭配檔案I2C_forArduino, 將其燒錄到arduino上
    2. 共地線不是必需的, 但有它信號會比較穩
    3. 燒錄esp32 cam的時候別插著其他線, 容易因為分壓導致燒錄失敗, 建議空載情況下燒錄
    4. 這個程式碼最好不要給其他的esp32 系列使用, 有必要請查閱esp32 I2C說明
    5. 經過測試, esp32 cam 只適合做為主機使用, 不適合當作從機, 死了這條心吧, bug劇多
    6. 傳輸訊號時要注意byte的形式為0xFF等十六進位的ascll code形式, 需要自行處理整數跟字元的接收形式
    7. 從機端需要同時擁有 receiveEvent requestEvent 來應對收發狀況, 不然一定是亂碼
*/
#include <Wire.h>  
#define I2C_SDA 12
#define I2C_SCL 13
#define boudrate 115200 

unsigned long previousMillis = 0;
const int interval = 2000;

void setup() {
  Serial.begin(boudrate);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.begin();
  Serial.println("Setup complete. Ready to send and receive I2C commands.");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (!transmeet()) {
      Serial.println("Error: Transmission failed. Check connection.");
    }
  }
}

bool transmeet(){
  Wire.beginTransmission(0x39);  
  if (Wire.write("hello \n") == 0) {
      return false; 
  }
  int data_1 = 128;
  Wire.write(lowByte(data_1));
  Wire.write(highByte(data_1));

  Wire.endTransmission();       

  Wire.requestFrom(0x39, 4);
  while (Wire.available() > 2){
    char c = Wire.read();    
    Serial.print(c); 
  }
  int data_2 =  (Wire.read() << 8) | Wire.read();
  Serial.print(" data: ");
  Serial.println(data_2);

  return true;
}

