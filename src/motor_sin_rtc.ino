// ============================================================
//  MOTOR 28BYJ-48 + DRIVER ULN2003 — SIN RTC
//  Arduino Nano | usa millis(), no bloquea el procesador
//
//  PINES:
//    IN1 → D8  |  IN2 → D10  |  IN3 → D9  |  IN4 → D11
//    VCC → 5V  |  GND → GND
// ============================================================

#include <AccelStepper.h>

// HALF4WIRE = medio paso, que es lo que necesita el 28BYJ-48
AccelStepper motor(AccelStepper::HALF4WIRE, 8, 10, 9, 11);

// ── Ajusta estos valores ──────────────────────────────────
const float DISTANCIA_CM   = 3.2;    // cm que quieres mover por ciclo
const float DIAMETRO_CM    = 20.41;  // diámetro de la polea en CM
const long  INTERVALO_MS   = 5000;   // cada cuánto mover (ms). 5000 = 5 seg
// ─────────────────────────────────────────────────────────

const int PASOS_POR_VUELTA = 2048;   // no tocar — es fijo para el 28BYJ-48

// Cálculo correcto (multiplicación primero para evitar truncamiento a 0)
const int PASOS = (int)( (DISTANCIA_CM * PASOS_POR_VUELTA) /
                         (3.14159 * DIAMETRO_CM) );

// Orden de pines: 8,10,9,11 — NO cambiar, el 28BYJ-48 lo requiere así
unsigned long ultimoMovimiento = 0;

void setup() {
  Serial.begin(9600);
  motor.setMaxSpeed(500);
  motor.setSpeed(200);

  Serial.println("=== Motor listo (sin RTC) ===");
  Serial.print("  Pasos por ciclo : "); Serial.println(PASOS);
  Serial.print("  Intervalo       : "); Serial.print(INTERVALO_MS / 1000); Serial.println(" seg");

  if (PASOS == 0) {
    Serial.println("[ERROR] Pasos = 0. Revisa DISTANCIA_CM y DIAMETRO_CM.");
    while (true);
  }
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultimoMovimiento >= INTERVALO_MS) {
    ultimoMovimiento = ahora;
    Serial.print("[MOTOR] Moviendo "); Serial.print(PASOS); Serial.println(" pasos...");
    motor.runSpeed();
    Serial.println("[MOTOR] Listo.");
  }
}
