//温度測定プログラム
#include <M5StickC.h>
#include "SHT3X.h"   //M5Stack用環境センサユニット ver.2
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "Ambient.h"

SHT3X sht30;        //M5Stack用環境センサユニット ver.2

#define uS_TO_S_FACTOR 1000000    // マイクロ秒から秒への変換係数
#define TIME_TO_SLEEP 600        // ESP32がスリープ状態になる時間（秒単位)

//関数プロトタイプ宣言
//void get_data(void);
void wifi_conect(void);
void ambient_access(void);
void lcd_display(void);

WiFiClient client;
Ambient ambient;

const char* ssid = "HUAWEI-F4E1";  //wifi名称
const char* password = "64141340"; //wifiパスワード

unsigned int channelId = 39225; // AmbientのチャネルID
const char* writeKey = "2c6c8657bd7cdc92"; // ライトキー

float temp = 0.0;                                 //温度変数
RTC_DATA_ATTR float hAveTemp = 0;                //1時間平均気温変数
float hum = 0.0;                                  //湿度変数
RTC_DATA_ATTR char flg = 5;                       //1時間経過フラグ

void setup() {
    M5.begin();
    M5.Axp.ScreenBreath(10);    // 画面の輝度を少し下げる
    M5.Lcd.setRotation(3);      // 左を上にする
    M5.Lcd.setTextSize(2);      // 文字サイズを2にする
    M5.Lcd.fillScreen(BLACK);   // 黒で塗りつぶす
    Wire.begin();               // I2Cを初期化する
}

void loop() 
{
    if(sht30.get() == 0)
    {
      temp = sht30.cTemp;               //温度取り込み
      //hum = sht30.humidity;             //湿度取り込み
    }
    
    //1時間平均気温計算
    if(hAveTemp == 0)   
    {
        hAveTemp = temp; 
    }
    else
    {
        hAveTemp = (hAveTemp+temp)/ 2;
    }
    
    if(flg == 5)  //一時間経過しているか又は初回
    {
      wifi_conect();      //wifi接続関数
      ambient_access();   //アンビエントアクセス関数
      //lcd_display();      //LCD表示関数
      WiFi.disconnect();  //wifiから切断
      hAveTemp = 0;      //1時間平均気温リセット
      flg = 0;            //フラグリセット
    }
    else
    {
      flg++;
    }
    esp_deep_sleep(TIME_TO_SLEEP * uS_TO_S_FACTOR ); //deepsleepに移行し、引数μ秒後に復帰する  
}


//wifi接続関数
void wifi_conect(void)
{
    WiFi.begin(ssid, password);               //  Wi-Fi APに接続 
    while (WiFi.status() != WL_CONNECTED)     //  Wi-Fi AP接続待ち
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());
}

//アンビエントアクセス関数
void ambient_access()
{
    if(sht30.get() == 0)
    {
      temp = sht30.cTemp;              //温度取り込み
      hum = sht30.humidity;           //湿度取り込み
    }
    
    ambient.begin(channelId, writeKey, &client); //チャネルIDとライトキーを指定してAmbientの初期化

    //Ambientに送信するデータをセット 
    ambient.set(1, temp);
    //ambient.set(2, hum);
    ambient.set(3, hAveTemp);
    
    ambient.send();                   //ambientにデータを送信
    
}

void lcd_display()
{
    //LCDにデータを表示
    M5.Lcd.setCursor(0, 0, 1);      //カーソル位置を変更
    M5.Lcd.printf("temp: %4.1f'C\r\n", temp);   //温度表示
    //M5.Lcd.printf("humid:%4.1f%%\r\n", hum);  //気圧表示
    M5.Lcd.printf("temp: %4.1f'C\r\n", hAveTemp);   //1時間平均温度
}
