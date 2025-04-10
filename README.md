# 🧠 지능형 링거 시스템 (Intelligent IV Bag Service)

## 📌 개요 (Overview)

이 프로젝트는 링거 투약 시 링거액의 잔량을 실시간으로 측정하고, 주입 속도 및 남은 시간을 계산하여 환자와 의료진에게 알림을 제공하는 **지능형 링거 모니터링 시스템**입니다.
HC-05 블루투스 모듈과 Firebase, Android 앱을 연동하여 **실시간 원격 모니터링**이 가능합니다.

## 🛠 사용한 기술 및 부품

- ATmega128 (MCU)
- Load Cell 센서 + HX711 모듈 (무게 측정)
- HC-05 Bluetooth 모듈 (무선 통신)
- Firebase (데이터 서버)
- Android Studio (앱 제작)
- 부저 (속도 이상 시 경보)

## 🧩 주요 기능

- 링거액 무게 실시간 측정
- 남은 용량 및 주입 속도 계산
- 남은 예상 시간 표시
- 속도 이상 시 경고음 울림
- Bluetooth로 스마트폰 앱에 데이터 전송
- Firebase 연동을 통한 클라우드 데이터 관리

## 🚀 실행 방법

1. LoadCell과 HX711을 ATmega128에 연결
2. 센서 값 측정 및 보정 후, 실시간 무게 계산
3. HC-05로 Bluetooth 통신 연결
4. Firebase에 데이터 업로드
5. Android 앱에서 실시간 데이터 확인

## 🙋‍♀️ 제작자

- **안예지 (Yeji Ahn)**  
  한국공학대학교 임베디드시스템 전공
  Email: yejilove9001@naver.com  
  GitHub: [yejilove9001](https://github.com/yejilove9001)
