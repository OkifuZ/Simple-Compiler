#pragma once

#ifndef INTERCODE
#define INTERCODE

#include <string>
#include <vector>
#include <iostream>
#include "errHand.h"
#include "tool.h"

enum class INT_OP{ERROR=-1, ADD=0, SUB, MULT, DIV, ASSIGN, 
                  SCAN, PRINT, J, EXIT, FUNC, 
                  ENDFUNC, ARRINI, 
                  BLE, BLT, BGE, BGT, BNE, BEQ, LABEL,
                  JAL, BEFCALL, ENDCALL, /*PARA,*/ PUSH,
                  RETURN};

class InterCodeEntry;
class InterOprand;

class Intermediate {
public:

    Intermediate(EnvTable *env_) : env(env_) {}
    
    // z = x op y
    void addInterCode(INT_OP op, std::string z, int z_type,
        std::string x, int x_type, bool isCon1, 
        std::string y, int y_type, bool isCon2); // if not valid, just pass ""

            // z = x op y
    void addInterCode(INT_OP op, std::string z, int z_type,
        std::string x, int x_type, bool isCon1, 
        std::string y, int y_type, bool isCon2,
        std::string rv, int rv_type, bool isCon3,
        bool arrInRight); // if not valid, just pass ""

    void addInterCode(INT_OP op, std::string z, int z_type,
        std::vector<int>& iniList); // if not valid, just pass ""


    void printInterCode(std::ostream& os);

    std::string nextTempVar();

    std::string nextLabel(std::string funcName, std::string tag = "");

    std::vector<InterCodeEntry*> interCodeList;

private:
    int temCount = 0;
    int funcLabelCount = 0;
    std::string lastFuncName = "";
    EnvTable* env;
    std::vector<std::string> INT_OP_STR = {"ADD", "SUB", "MULT", "DIV", "ASSIGN", 
                                           "SCAN", "PRINT", "J", "EXIT", "FUNC", 
                                           "ENDFUNC", "ARRINI",
                                           "BLE", "BLT", "BGE", "BGT", "BNE", "BEQ", "LABEL",
                                           "JAL", "BEFCALL", "ENDCALL", /*"PARA",*/ "PUSH",
                                           "RETURN"};

};


class InterOprand {
public:
    std::string name;
    int type;
    bool isCon;
    bool isValid;
    bool isIntermediate;

    InterOprand(std::string name_, int type_, bool isCon_) : name(name_), type(type_), isCon(isCon_) {
        isValid = !(name == "");
        isIntermediate = (name[0] == '#');
    }

    int getConstInt() {
        if (isCon && type == _TYPE_INT) {
            return str2int(name);
        } 
        return -1;
    }

    char getConstChar() {
        if (isCon && type == _TYPE_CHAR) {
            return static_cast<char>(str2int(name));
        }
        return '\0';
    }

};

class InterCodeEntry {
public:
    INT_OP op;
    InterOprand* x;
    InterOprand* y;
    InterOprand* z;

    InterCodeEntry(INT_OP op_, InterOprand* x_, InterOprand* y_, InterOprand* z_) : op(op_), x(x_), y(y_), z(z_) {}
    virtual ~InterCodeEntry() = default;
    
};

class InterCodeEntry_array : public InterCodeEntry {
public:
    InterOprand* rv;
    bool arrInRight;
    InterCodeEntry_array(INT_OP op_, InterOprand* x_, InterOprand* y_, InterOprand* z_, InterOprand* rv_, bool arrInRight_) :
       InterCodeEntry(op_, x_, y_, z_), rv(rv_), arrInRight(arrInRight_) {}
};

class InterCodeEntry_arrDec : public InterCodeEntry {
public:
    std::vector<int>& iniList;
    InterCodeEntry_arrDec(INT_OP op_, InterOprand* x_, InterOprand* y_, InterOprand* z_, std::vector<int>& v) :
        InterCodeEntry(op_, x_, y_, z_), iniList(v) {}
};









#endif // INTERCODE
