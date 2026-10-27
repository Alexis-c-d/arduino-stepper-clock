# Arduino Nano Stepper Clock

Arduino Nano project combining a **28BYJ-48 stepper motor**, **DS1302 RTC**, **Hall sensor**, and **buzzer** for a timed motion system.

---

## Hardware
| Component | Pins | Library |
|-----------|------|---------|
| 28BYJ-48 + ULN2003 | IN1→D8, IN2→D9, IN3→D10, IN4→D11 | `AccelStepper` (half-step, 4096 steps/rev) |
| DS1302 RTC | DAT→D6, CLK→D5, RST→D7 | `Rtc by Makuna` |
| Hall sensor (A3144) | OUT→D2 | — |
| Active buzzer | +→D12 | — |

---

## Repository Layout
```
arduino-stepper-clock/
├── src/
│   ├── motor_rtc_hall_buzzer.ino      # Full version: RTC + Hall + buzzer
│   ├── motor_rtc_hall_buzzer_v2.ino   # Variant 2
│   └── motor_sin_rtc.ino              # Version without RTC
├── docs/
│   └── wiring_diagram.png  # (to be added)
└── README.md
```

---

## Building & Uploading
1. Install libraries in Arduino IDE **Library Manager**:
   - `AccelStepper` by Mike McCauley
   - `Rtc by Makuna`
2. Open desired `.ino` in Arduino IDE.
3. **First upload only**: set `#define SINCRONIZAR_HORA 1` to sync RTC with PC time.
4. Upload, open Serial Monitor (9600 baud), wait for sync message.
5. Change `SINCRONIZAR_HORA` to `0` and re-upload.

---

## Configuration (edit at top of `.ino`)
```cpp
const float    DISTANCIA_CM  = 3.2;   // Linear travel per cycle
const float    DIAMETRO_CM   = 20.41; // Pulley diameter
const uint32_t INTERVALO_SEG = 5;     // Seconds between moves
```
Steps per cycle calculated automatically:
```cpp
const int PASOS = (DISTANCIA_CM * 4096) / (PI * DIAMETRO_CM);
```

---

## Operation
- **Hall sensor** detects magnet → buzzer sounds, logs to Serial.
- **RTC** triggers motor move every `INTERVALO_SEG` seconds.
- **AccelStepper** runs move with acceleration (400 steps/s², max 400 steps/s).
- Serial debug prints: `[MOTOR] Moving X steps...`, `[HALL] Magnet detected.`

---

## Extracted From
Arduino sketches found in `Downloads/`:
- `motor_rtc_hall_buzzer.ino`
- `motor_rtc_hall_buzzer_v2.ino`
- `motor_sin_rtc.ino`

---

## License
MIT