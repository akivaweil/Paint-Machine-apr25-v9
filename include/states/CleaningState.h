#ifndef CLEANING_STATE_H
#define CLEANING_STATE_H

#include "State.h"

class CleaningState : public State {
public:
    CleaningState();
    void enter() override;
    void update() override;
    void exit() override;
};

#endif // CLEANING_STATE_H 