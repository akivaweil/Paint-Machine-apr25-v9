#ifndef PAUSED_STATE_H
#define PAUSED_STATE_H

#include "State.h"

class PausedState : public State {
public:
    PausedState();
    void enter() override;
    void update() override;
    void exit() override;
};

#endif // PAUSED_STATE_H 