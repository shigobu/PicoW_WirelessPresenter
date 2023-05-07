#include <KeyboardBLE.h>
#include <pico/cyw43_arch.h>

#define SW_OFF HIGH
#define SW_ON LOW

const int LEFT_SW_PIN = 1;
const int RIGHT_SW_PIN = 5;
const int SW_ON_THRESHOLD = 10;
const int ADC_MAX = 4095;
int lastBatteryLevel = 100;   //%
int batteryLevelCount = 0;
int currentBatteryLevel = 0;

void setup() {
  pinMode(LEFT_SW_PIN, INPUT_PULLUP);
  pinMode(RIGHT_SW_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  analogReadResolution(12);

  KeyboardBLE.begin();
  delay(5000);
}

void loop() {
  //チャタリング対策。SW_ON_THRESHOLDミリ秒ONが続いたものを採用する。
  int rightCount = 0;
  while (digitalRead(RIGHT_SW_PIN) == SW_ON){
    rightCount++;
    delay(1);
    if (rightCount > SW_ON_THRESHOLD){
      break;
    }
  }
  if (rightCount > SW_ON_THRESHOLD) {
    KeyboardBLE.write(KEY_RIGHT_ARROW);
    while (digitalRead(RIGHT_SW_PIN) == SW_ON);
  }

  int leftCount = 0;
  while (digitalRead(LEFT_SW_PIN) == SW_ON){
    leftCount++;
    delay(1);
    if (leftCount > SW_ON_THRESHOLD){
      break;
    }
  }
  if (leftCount > SW_ON_THRESHOLD) {
    KeyboardBLE.write(KEY_LEFT_ARROW);
    while (digitalRead(LEFT_SW_PIN) == SW_ON);
  }

  //電池残量
  if (batteryLevelCount < 100) {
    currentBatteryLevel += GetBatteryLevel();
    batteryLevelCount++;
  }
  else {
    int tempLevel = currentBatteryLevel / 100;
    currentBatteryLevel = 0;
    batteryLevelCount = 0;
    if (lastBatteryLevel != tempLevel){
      KeyboardBLE.setBattery(tempLevel);
      lastBatteryLevel = tempLevel;
    }
  }
}

int GetBatteryLevel(){
  //cyw43_arch_wait_for_work_until(make_timeout_time_ms(10));
  //pinMode(A3, INPUT);
  //pinMode(25, OUTPUT);
  //digitalWrite(25, HIGH);
  //10回取得して平均を取る
  int count = 10;
  int analogVal = 0;
  for (int i = 0; i < count; i++){
    int temp = 0;
    while (true){
      cyw43_arch_wait_for_work_until(make_timeout_time_ms(10));
      pinMode(A3, INPUT);
      pinMode(25, OUTPUT);
      digitalWrite(25, HIGH);
      temp = analogRead(A3);
      if (temp < 700){    //1.8V以下の場合は不正な値と見る。サンプリングしない。
        continue;
      }
      break;
    }
    analogVal += temp;
    delay(1);
  }
  analogVal = analogVal / count;
  double voltage = mapf(analogVal, 0, ADC_MAX, 0, 3.3) * 3;   //1/3の値がA3に入るので、3倍してもとの値に戻す。
  int batteryLevel = mapf(voltage, 2, 3, 0, 100);
  return constrain(batteryLevel, 0, 100);
}

double mapf(double x, double in_min, double in_max, double out_min, double out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}