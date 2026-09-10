#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

enum CurtainState { CLOSED, OPEN }; // Possible states of the curtain

class MotorController {
public:
  MotorController(int stepPin, int dirPin, int enablePin);
  void begin();
  void moveTo(long targetPosition, CurtainState newState, void (*yieldCallback)() = nullptr); // move curtain to target position. yieldCallback allows background network tasks to run during motion
  
  CurtainState getState() const { return m_state; }
  long getPosition() const { return m_currentPosition; }

private:
  int m_stepPin;
  int m_dirPin;
  int m_enablePin;
  long m_currentPosition;
  CurtainState m_state;
};

#endif // MOTOR_CONTROLLER_H