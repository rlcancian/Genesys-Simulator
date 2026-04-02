#pragma once

#include "State.h"

/*
class State_Numerical: public State {
public:
    State_Numerical(long value)
            : State(ColorCA(0,0,0), ' ', value, ""){
                this->longValue = value;
            }
    State_Numerical(const State_Numerical& orig)
    :State(orig) {
        
    }
    virtual ~State_Numerical() = default;
public:
    virtual ColorCA getColor() const override {
        int llongValue = 0;
        if (longValue)
            llongValue = 1<<9;
        uint8_t south = longValue&((1<<3)|(1<<5)|(1<<7))?0:255;
        uint8_t r = longValue&((1<<0)|(1<<4)|(1<<5))?0:south; // west : south
        uint8_t g = longValue&((1<<1)|(1<<6)|(1<<7))?0:south; // east : south
        uint8_t b = longValue&((1<<2)|(1<<4)|(1<<6))?0:south; // north : south
        //std::cout << longValue << ": "<< (int)r << ","<<(int)g<<","<<(int)b<<std::endl;
        return ColorCA(r,g,b);
    }

protected:
private:
};
*/
