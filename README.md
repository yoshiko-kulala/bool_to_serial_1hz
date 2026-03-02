dev
```
sudo chmod 777 /dev/ttyACM0
```

kidou
```
ros2 launch bool_to_serial_1hz bool_to_serial_1hz.launch.py port:=/dev/ttyACM0 baud:=115200 topic:=/to_arduino
```
or
```
ros2 launch bool_to_serial_1hz bool_to_serial_1hz.launch.py
```

arduino側  
```
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

test on
```
ros2 topic pub /to_arduino std_msgs/msg/Bool "{data: true}" -1
```
test off
```
ros2 topic pub /to_arduino std_msgs/msg/Bool "{data: false}" -1
```
