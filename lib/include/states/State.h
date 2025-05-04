#ifndef STATE_H
#define STATE_H

class State {
public:
    virtual ~State() {}
    virtual void enter() = 0;
    virtual void update() = 0;
    virtual void exit() = 0;
};

#endif // STATE_H 