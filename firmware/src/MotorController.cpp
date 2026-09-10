#include "MotorController.h"

extern void logMessage(String msg);

MotorController::MotorController(int stepPin, int dirPin, int enablePin)
  : m_stepPin(stepPin),
    m_dirPin(dirPin),
    m_enablePin(enablePin),
    m_currentPosition(POSITION_CLOSED),
    m_state(CLOSED) {}

void MotorController::begin() {
  pinMode(m_stepPin, OUTPUT);
  pinMode(m_dirPin, OUTPUT);
  pinMode(m_enablePin, OUTPUT);
  digitalWrite(m_enablePin, HIGH); // Disable driver on boot to eliminate idle coil heat
}

// Prevent driving past the closed position (0 steps) to avoid damaging the curtain.
void MotorController::moveTo(long targetPosition, CurtainState newState, void (*yieldCallback)()) {
  if (targetPosition < POSITION_CLOSED) {
    targetPosition = POSITION_CLOSED;
  }

  // Prevent driving past the open position (max steps) to avoid damaging the curtain.
  if (m_currentPosition == targetPosition) {
    logMessage(">> Already at target position. Command ignored.");
    return;
  }

  long totalSteps = abs(targetPosition - m_currentPosition);
  
  // Direction mapping: 0 for Open (CW/Left), 1 for Close (CCW/Right)
  int dir = (targetPosition > m_currentPosition) ? 0 : 1;
  digitalWrite(m_dirPin, dir);

  logMessage(">> Moving curtain " + String(totalSteps) + " steps (Dir: " + String(dir) + ")...");

  // Energize coils and stabilize driver current before starting motion
  digitalWrite(m_enablePin, LOW); // Energize stepper coils
  delay(10);

  // Trapezoidal acceleration/deceleration profile for smooth motion
  for (long step = 0; step < totalSteps; step++) {
    int currentDelay = STEP_MIN_DELAY_US;

    // Smooth Start Ramp: shrink delay from MAX to MIN over the first STEP_RAMP_STEPS
    if (step < STEP_RAMP_STEPS) {
      currentDelay = map(step, 0, STEP_RAMP_STEPS, STEP_MAX_DELAY_US, STEP_MIN_DELAY_US);
    } 
    // Smooth End Cushion: expand delay from MIN to MAX over the last STEP_RAMP_STEPS
    else if (step > (totalSteps - STEP_RAMP_STEPS)) {
      currentDelay = map(step, totalSteps - STEP_RAMP_STEPS, totalSteps, STEP_MIN_DELAY_US, STEP_MAX_DELAY_US);
    }
    // Single step pulse to move the motor
    digitalWrite(m_stepPin, HIGH);
    delayMicroseconds(currentDelay);
    digitalWrite(m_stepPin, LOW);
    delayMicroseconds(currentDelay);

    // Yield control periodically to service network requests during long travel
    if (yieldCallback && (step % 150 == 0)) {
      yieldCallback();
    }
  }

  digitalWrite(m_enablePin, HIGH); // De-energize coils to prevent driver overheating

  m_currentPosition = targetPosition;
  m_state = newState;
  logMessage(">> Motion complete. Position: " + String(m_currentPosition) + 
             " | State: " + String((m_state == OPEN) ? "OPEN" : "CLOSED"));
}