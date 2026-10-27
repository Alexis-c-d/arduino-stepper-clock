/*******************************************************************************
 * Arduino Nano Stepper Clock - 28BYJ-48 + RTC DS1302 + Hall Sensor + Buzzer
 *
 * Hardware:
 * - Arduino Nano
 * - 28BYJ-48 Stepper Motor (4096 steps/rev, half-step mode) + ULN2003 driver
 * - DS1302 RTC Module
 * - Hall Effect Sensor (A3144 or similar)
 * - Active Buzzer
 *
 * Pinout:
 *   Motor IN1-IN4 -> D8, D9, D10, D11
 *   RTC DAT/CLK/RST -> D6, D5, D7
 *   Hall Sensor OUT -> D2
 *   Buzzer + -> D12
 *
 * Libraries required:
 * - AccelStepper (Mike McCauley)
 * - Rtc by Makuna (for DS1302)
 *
 * Autor: Luis Alexis Calvillo Deveze
 * Fecha: 2026
 *******************************************************************************/

#include <AccelStepper.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ============================================================================
// CONFIGURATION
// ============================================================================
// Set to 1 ONLY the first time to sync RTC with computer time
// After first upload, change to 0 and re-upload
#define SINCRONIZAR_HORA  0

// Pin definitions
#define PIN_HALL    2
#define PIN_BUZZER  12

// Mechanical parameters (adjust for your setup)
const float    DISTANCIA_CM  = 3.2;   // Linear distance per cycle (cm)
const float    DIAMETRO_CM   = 20.41; // Pulley/wheel diameter (cm)
const uint32_t INTERVALO_SEG = 5;     // Time between movements (seconds)

const int PASOS_POR_VUELTA = 4096;  // 28BYJ-48 in half-step mode
const int PASOS = (DISTANCIA_CM * PASOS_POR_VUELTA) / (3.141592 * DIAMETRO_CM);

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
ThreeWire myWire(6, 5, 7);              // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);
AccelStepper motor(AccelStepper::HALF4WIRE, 8, 9, 10, 11);  // IN1, IN2, IN3, IN4

// State variables
uint32_t ultimoGiro = 0;
bool moviendose = false;
bool imanAnterior = false;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    Serial.begin(9600);

    pinMode(PIN_HALL, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    // Motor configuration
    motor.setMaxSpeed(400);
    motor.setAcceleration(200);
    motor.enableOutputs();

    // RTC initialization
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
        Serial.println("[ERROR] Fecha del RTC invalida.");
        Serial.println("        Pon SINCRONIZAR_HORA en 1, sube y luego ponlo en 0.");
        while (true);
    }

    ultimoGiro = ahora.Epoch32Time();

    Serial.println("=== Sistema listo ===");
    Serial.print("  Pasos por ciclo : "); Serial.println(PASOS);
    Serial.print("  Intervalo       : "); Serial.print(INTERVALO_SEG); Serial.println(" seg");

    if (PASOS == 0) {
        Serial.println("[ERROR] Pasos = 0. Revisa DISTANCIA_CM y DIAMETRO_CM.");
        while (true);
    }
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
    // ---- Hall sensor monitoring (edge detection) ----
    bool imanActual = (digitalRead(PIN_HALL) == HIGH);

    if (imanActual != imanAnterior) {
        imanAnterior = imanActual;
        if (imanActual) {
            digitalWrite(PIN_BUZZER, HIGH);
            Serial.println("[HALL] Iman detectado.");
        } else {
            digitalWrite(PIN_BUZZER, LOW);
            Serial.println("[HALL] Iman retirado.");
        }
    }

    // ---- Motor control with RTC timing ----
    RtcDateTime ahora = rtc.GetDateTime();

    if (!ahora.IsValid()) {
        Serial.println("[ERROR] Lectura de RTC invalida, reintentando...");
        return;
    }

    uint32_t tiempoActual = ahora.Epoch32Time();

    // Check if it's time for next movement
    if (!moviendose && (tiempoActual - ultimoGiro >= INTERVALO_SEG)) {
        Serial.print("[MOTOR] Moviendo "); Serial.print(PASOS); Serial.println(" pasos...");
        motor.enableOutputs();
        motor.move(PASOS);
        moviendose = true;
    }

    // Run motor
    if (moviendose) {
        motor.run();

        if (motor.distanceToGo() == 0) {
            moviendose = false;
            ultimoGiro = tiempoActual;
            Serial.println("[MOTOR] Listo.");
            apagarMotor();
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
void apagarMotor() {
    motor.disableOutputs();
}