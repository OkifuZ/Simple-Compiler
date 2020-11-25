#pragma once

#ifndef MIPSGENE
#define MIPSGENE

#include <string>
#include <vector>
#include <map>
#include <set>
#include "InterCode.h"
#include "tool.h"
#include "parser.h"
#include "errHand.h"
#include "register.h"

enum class MIPS_INS{
    ERROR=-1, ADDU=0, ADDIU, SUBU, LI, LA, MOVE, 
    MUL, DIVU, MFLO, MFHI,
    J,
    LW, SW,
    SYSCALL, 
    DATASEG, TEXTSEG, STRINGSEG, SPACE, LABEL, 
    BNE, BEQ, BLE, BLT, BGE, BGT, JR,
    DEBUG
};


class MipsEntry {
public:
    MIPS_INS op;
    std::string x = "";
    std::string y = "";
    std::string z = "";
    int immediate = -1;
    bool hasImmediate = false;
    MipsEntry(MIPS_INS op_, std::string z_, std::string x_, std::string y_, int immediate_, bool hasImmediate_) : 
        op(op_), x(x_), y(y_), z(z_), immediate(immediate_), hasImmediate(hasImmediate_) {}
};



class MipsGenerator {
public:
    MipsGenerator(Parser* p) {
        interCodeList = p->intermediate->interCodeList;
        stringList = p->globalStringList;
        env = p->envTable;

        for (int i = 0; i < 8; i++) {
            globalRegister.push_back(Register("$s"+int2str(i)));
        }
        for (int i = 0; i < 10; i++) {
            temRegister.push_back(Register("$t"+int2str(i)));
        }
        for (int i = 0; i < 4; i++) {
            aRegister.push_back(Register("$a"+int2str(i)));
        }
        for (int i = 0; i < 2; i++) {
            vRegister.push_back(Register("$v"+int2str(i)));
        }

    }

    void GeneMipsCode();

    void printMipsCode(std::ostream& os);

    SymTableEntry* getSymByName(std::string name);

    bool checkIsConst(std::string name, int* value);

    void freeReg(std::string regName);

private:
    void addEntry(MipsEntry* e) { mipsCodeList.push_back(e); }

    std::string getRegister(std::string varName, bool load);

    void pushStack(std::string reg);

    void loadValue(std::string reg, int offset);

    int getStringIndex(std::string s) {
        for (int i = 0; i < stringList.size(); i++) {
            if (stringList[i] == s) {
                return i;
            }
        }
        return -1;
    }


    std::string getEmptyTemReg() {
        for (int i = 0; i < 10; i++) {
            if (!temRegister[i].isBusyL && !temRegister[i].isBusyT) {
                return "$t"+int2str(i);
            }
        }
        return "";
    }

    std::string getTemRegByName(std::string name) {
        for (int i = 0; i < 10; i++) {
            if (temRegister[i].varName == name) {
                return "$t"+int2str(i);
            }
        }
        return "";
    }

    std::string getGloRegByName(std::string name) {
        for (int i = 0; i < 8; i++) {
            if (globalRegister[i].varName == name) {
                return "$s"+int2str(i);
            }
        }
        return "";
    }
    int prevPos = 0;
    std::string graspTemReg();

    void storeBack(std::string reg, bool allocate, bool fake);
    void loadValue(std::string name, std::string reg);
    int varInTemRegister(std::string name);
    int varInGloRegister(std::string name);
    int varInARegister(std::string name);

    void assignGloReg2LocVar(SymbolTable* sym);
    std::string getNameByReg(std::string reg);

    void storeGloRegOfCaller(SymbolTable* symTab);
    int callerLocalVarInReg = 0;
    void restoreGloRegOfCaller(SymbolTable* symTab);
    void storeCallerTemReg();
    void restoreCallerTemReg();
    void storeCallerAReg();
    void restoreCallerAReg();

    void pushReg(std::string reg, bool fake false);
    void popReg(std::string reg);

    std::string curFuncName = "global";
    int topOffset = 0; // always >= 0

    std::vector<InterCodeEntry*> interCodeList;
    std::vector<MipsEntry*> mipsCodeList;
    std::vector<std::string> stringList;
    EnvTable env;
    std::map<std::string, int> temVarOffsetMap; // temVarName : offset

    std::vector<Register> globalRegister;
    std::vector<Register> temRegister;
    std::vector<Register> vRegister;
    std::vector<Register> aRegister;
    


    std::set<std::string> usingInCurInter;

};




#endif // MIPSGENE