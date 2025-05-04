#ifndef MANUAL_MOVE_STATE_H
#define MANUAL_MOVE_STATE_H

#include "states/State.h" // Assuming a base State class exists

// Forward declarations
class ServoMotor; 

class ManualMoveState : public State {
public:
    ManualMoveState();
    void enter() override;
    void update() override;
    void exit() override;

    // Method to handle the actual move command
    void moveToPosition(long targetX, long targetY, long targetZ, int targetAngle);

private:
    // Add any private members if needed
    bool isMoving; 
};

#endif // MANUAL_MOVE_STATE_H 