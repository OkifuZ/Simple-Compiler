#pragma once
#ifndef REGISTER
#define REGISTER

#include <string>
#include <vector>
#include "tool.h"

class Register {
public:
    int state;
    std::string regName;
    int index;
    bool isT;
    bool isS;
    bool isV;
    bool isA;

    std::string varName = "";
    bool isBusyT = false;
    bool isBusyL = false;

    Register(std::string reg) {
        regName = reg;
        isS = reg[1] == 's';
        isV = reg[1] == 'v';
        isA = reg[1] == 'a';
        isT = reg[1] == 't';
        index = str2int(reg.substr(2, 1));
    }

    void setVar(std::string name) {
        this->setFree();
        if (name[0] == '#') {
            isBusyT = true;
        }
        else {
            isBusyL = true;
        }
        varName = name;
    }

    void setFree() {
        isBusyL = false;
        isBusyT = false;
        varName = "";
    }

};














#endif // REGISTER