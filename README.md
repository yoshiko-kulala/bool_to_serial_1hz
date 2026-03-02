# bool_to_serial_1hz

ROS 2 の `std_msgs/msg/Bool` を購読し、Arduino に `0/1` をシリアル送信するノードです。
Arduino 側では受信値に応じて 13 番ピンの LED を ON/OFF します。

## 1. 事前準備（dev）

シリアルデバイスにアクセスできるように権限を設定します。

```bash
sudo chmod 777 /dev/ttyACM0
```

## 2. ノード起動（kidou）

パラメータを明示して起動する場合:

```bash
ros2 launch bool_to_serial_1hz bool_to_serial_1hz.launch.py port:=/dev/ttyACM0 baud:=115200 topic:=/to_arduino
```

デフォルト値のまま起動する場合:

```bash
ros2 launch bool_to_serial_1hz bool_to_serial_1hz.launch.py
```

## 3. Arduino 側スケッチ（arduino側）

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '1') {
      digitalWrite(13, HIGH);
    } else if (c == '0') {
      digitalWrite(13, LOW);
    }
  }
}
```

## 4. 動作確認（test）

LED を ON（`true` を publish）:

```bash
ros2 topic pub /to_arduino std_msgs/msg/Bool "{data: true}" -1
```

LED を OFF（`false` を publish）:

```bash
ros2 topic pub /to_arduino std_msgs/msg/Bool "{data: false}" -1
```
