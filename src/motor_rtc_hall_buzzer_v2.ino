// ============================================================
//  MOTOR 28BYJ-48 + RTC DS1302 + SENSOR HALL + BUZZER
//  Arduino Nano | AccelStepper con medio paso (half step)
//
//  INSTALAR en Library Manager:
//    - "AccelStepper" de Mike McCauley
//    - "Rtc by Makuna"
//
//  PINES MOTOR:
//    IN1 → D8  |  IN2 → D9  |  IN3 → D10  |  IN4 → D11
//    VCC → 5V  |  GND → GND
//
//  PINES RTC:
//    DAT → D6  |  CLK → D5  |  RST → D7
//    VCC → 5V  |  GND → GND
//
//  PINES SENSOR HALL:
//    OUT → D2  |  VCC → 5V  |  GND → GND
//
//  PINES BUZZER (activo):
//    +   → D12 |  -   → GND
//
//  PRIMERA VEZ: pon SINCRONIZAR_HORA en 1, sube, espera el
//  mensaje en el monitor serie, luego ponlo en 0 y vuelve a subir.
// ============================================================

#include <AccelStepper.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ── Pon en 1 solo la primera vez para sincronizar la hora ──
#define SINCRONIZAR_HORA  0
// ──────────────────────────────────────────────────────────

#define PIN_HALL   2
#define PIN_BUZZER 12

// ── Ajusta estos valores ──────────────────────────────────
const float    DISTANCIA_CM  = 3.2;
const float    DIAMETRO_CM   = 20.41;
const uint32_t INTERVALO_SEG = 5;
// ─────────────────────────────────────────────────────────

const int PASOS_POR_VUELTA = 2048;
const int PASOS = (DISTANCIA_CM * PASOS_POR_VUELTA) / (3.141592 * DIAMETRO_CM);

ThreeWire myWire(6, 5, 7);
RtcDS1302<ThreeWire> rtc(myWire);

AccelStepper motor(AccelStepper::HALF4WIRE, 8, 9, 10, 11);

uint32_t ultimoGiro       = 0;
uint32_t ultimaLecturaRTC = 0;  // para leer el RTC solo cada 1 seg, no en cada loop
bool moviendose           = false;
bool imanAnterior         = false;

void apagarMotor() {
  motor.disableOutputs();
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_HALL, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  motor.setMaxSpeed(800);
  motor.setAcceleration(1000);
  motor.enableOutputs();

  rtc.Begin();

  if (!rtc.GetIsRunning()) {
    Serial.println("[RTC] Reloj detenido, arrancando...");
    rtc.SetIsRunning(true);
  }

#if SINCRONIZAR_HORA == 1
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  rtc.SetDateTime(compiled);
  Serial.println("[RTC] Hora sincronizada con la computadora.");
  Serial.println("      >>> Ahora pon SINCRONIZAR_HORA en 0 y vuelve a subir. <<<");
#endif

  RtcDateTime ahora = rtc.GetDateTime();
  if (!ahora.IsValid()) {
    Serial.println("[ERROR] Fecha del RTC inválida.");
    Serial.println("        Pon SINCRONIZAR_HORA en 1, sube y luego ponlo en 0.");
    while (true);
  }

  ultimoGiro       = ahora.Epoch32Time();
  ultimaLecturaRTC = ahora.Epoch32Time();

  Serial.println("=== Sistema listo ===");
  Serial.print("  Pasos por ciclo : "); Serial.println(PASOS);
  Serial.print("  Intervalo       : "); Serial.print(INTERVALO_SEG); Serial.println(" seg");

  if (PASOS == 0) {
    Serial.println("[ERROR] Pasos = 0. Revisa DISTANCIA_CM y DIAMETRO_CM.");
    while (true);
  }
}

void loop() {
  // ── 1. Motor: corre en CADA iteración del loop sin interrupciones ──
  if (moviendose) {
    motor.run();
    if (motor.distanceToGo() == 0) {
      moviendose = false;
      Serial.println("[MOTOR] Listo.");
      apagarMotor();
    }
  }

  // ── 2. Sensor Hall: solo detecta cambios, sin Serial dentro ─────
  bool imanActual = (digitalRead(PIN_HALL) == LOW); // cambia a HIGH si tu sensor es al revés
  if (imanActual != imanAnterior) {
    imanAnterior = imanActual;
    digitalWrite(PIN_BUZZER, imanActual ? LOW : HIGH);
    // Serial solo cuando cambia el estado, no en cada loop
    Serial.println(imanActual ? "[HALL] Iman detectado." : "[HALL] Iman retirado.");
  }

  // ── 3. RTC: se lee solo una vez por segundo, no en cada loop ────
  unsigned long ahoraMs = millis();
  static unsigned long ultimoCheckMs = 0;

  if (ahoraMs - ultimoCheckMs >= 1000) {
    ultimoCheckMs = ahoraMs;

    RtcDateTime ahora = rtc.GetDateTime();
    if (!ahora.IsValid()) {
      Serial.println("[ERROR] RTC invalido. Sincroniza la hora.");
      return;
    }

    uint32_t tiempoActual = ahora.Epoch32Time();

    if (!moviendose && (tiempoActual - ultimoGiro >= INTERVALO_SEG)) {
      ultimoGiro = tiempoActual;
      Serial.print("[MOTOR] Moviendo "); Serial.print(PASOS); Serial.println(" pasos...");
      motor.enableOutputs();
      motor.move(PASOS);
      moviendose = true;
    }
  }
}
