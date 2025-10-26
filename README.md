# IOT project Assignment

Anthony from CAU, Seoul, South Korea

## 코드 흐름 개요
- 웹 페이지(`pi/index.html`) → CGI 스크립트(`pi/sender.sh`) → C 프로그램(`pi/main`) → 아두이노(`arduino/arduino.ino`) 순서로 명령이 전달된다.
- `camera`, `segment`, `lcd` 세 가지 명령을 직렬 포트(`/dev/ttyACM0`)로 보내고, 아두이노는 명령 종류에 맞춰 LCD, 7-세그먼트, 부저를 제어한다.
- 카메라 촬영은 라즈베리 파이에서 `v4l2-ctl`을 실행해 `pi/capture.jpg`를 덮어쓰고, 웹 페이지는 최신 이미지를 다시 로드한다.

## 라즈베리 파이 측 구성
- `pi/index.html`/`pi/style.css`: LCD 텍스트, 7-세그먼트 숫자, 카메라 촬영 버튼을 제공하는 단일 페이지 UI. `fetch` 호출로 `sender.sh`를 호출한다.
- `pi/sender.sh`: CGI 또는 CLI에서 실행 가능. `mode`와 `param`을 파싱해 `sudo /var/www/html/main`을 호출하며, 촬영 시에는 `camera True`를 전달해 C 프로그램이 카메라 캡처를 수행하도록 한다.
- `pi/main.c` (`pi/main`): `/dev/ttyACM0`를 9600bps로 열고 `{mode} {param}\r\n` 포맷으로 아두이노에 전달한다. `camera True`일 때는 `v4l2-ctl` 명령을 실행해 사진을 저장한 뒤 직렬 메시지를 보낸다.

## 아두이노 스케치 흐름 (`arduino/arduino.ino`)
- `setup()`: LCD 초기화, 부저 핀 설정, 직렬 통신(9600bps) 시작 후 준비 메시지 출력.
- `loop()`: 직렬 버퍼에서 한 줄을 읽고 `camera`/`segment`/`lcd` 접두사에 따라 분기한다.
  - `camera`: 간단한 부저 멜로디(`tone`) 재생 후 `Camera Captured` 로그 출력.
  - `segment <digit>`: 7-세그먼트 핀(2–8번)에 숫자 패턴을 설정하고 시리얼 로그 남김.
  - `lcd <text>`: 첫 줄에 학번/이름, 둘째 줄에 전달받은 텍스트를 표시하고 로그 남김.
