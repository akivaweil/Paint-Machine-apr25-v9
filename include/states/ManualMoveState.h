#ifndef MANUAL_MOVE_STATE_H
#define MANUAL_MOVE_STATE_H

#include "states/State.h" // Assuming a base State class exists
#include <FastAccelStepper.h> // Added include

// Forward declarations
class ServoMotor; 

class ManualMoveState : public State {
public:
    ManualMoveState();
    void enter() override;
    void update() override;
    void exit() override;
    const char* getName() const override;

    // Method to handle the actual move command
    void moveToPosition(long targetX, long targetY, long targetZ, int targetAngle);

    // Public methods to trigger movement from external sources (e.g., web commands)
    void moveX(long relative_steps);

private:
    // Add any private members if needed
    bool isMoving; 
};

#endif // MANUAL_MOVE_STATE_H 