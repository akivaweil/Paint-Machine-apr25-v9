#ifndef PAINTING_STATE_H
#define PAINTING_STATE_H

#include "State.h"

class PaintingState : public State {
public:
    PaintingState();
    void enter() override;
    void update() override;
    void exit() override;
    const char* getName() const override;
};

#endif // PAINTING_STATE_H 