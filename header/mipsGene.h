#pragma once

#ifndef MIPSGENE
#define MIPSGENE

#include <string>
#include <vector>
#include <map>
#include "InterCode.h"
#include "tool.h"
#include "parser.h"
#include "errHand.h"

enum class MIPS_INS{
    ERROR=-1, ADDU=0, ADDIU, SUBU, LI, LA, MOVE, 
    MUL, DIVU, MFLO, MFHI,
    J,
    LW, SW,
    SYSCALL, 
    DATASEG, TEXTSEG, STRINGSEG, LABEL
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

class RegistorPool {
public:
    std::vector<std::string> globalRegister = std::vector<std::string>(8, "");
    std::vector<std::string> temRegister = std::vector<std::string>(10, "");
    std::vector<std::string> vRegister = std::vector<std::string>(2, "");
    std::vector<std::string> aRegister = std::vector<std::string>(4, "");

    std::string getEmptyTemReg() {
        for (int i = 0; i < 8; i++) {
            if (temRegister[i] == "") {
                return "$"+int2str(i);
            }
        }
        return "";
    }

    std::string getTemRegByName(std::string name) {
        for (int i = 0; i < 8; i++) {
            if (temRegister[i] == name) {
                return "$"+int2str(i);
            }
        }
        return "";
    }

    std::string graspTemReg() {
        return "$t0";
    }

};


class MipsGenerator {
public:
    MipsGenerator(Parser* p) {
        interCodeList = p->intermediate->interCodeList;
        stringList = p->globalStringList;
        env = p->envTable;
    }

    void GeneMipsCode();

    void printMipsCode(std::ostream& os);

    void addEntry(MipsEntry* e) { mipsCodeList.push_back(e); }

    std::string getRegister(std::string varName);

    int getStringIndex(std::string s) {
        for (int i = 0; i < stringList.size(); i++) {
            if (stringList[i] == s) {
                return i;
            }
        }
        return -1;
    }

private:
    std::vector<InterCodeEntry*> interCodeList;
    std::vector<MipsEntry*> mipsCodeList;
    std::vector<std::string> stringList;
    RegistorPool regPool;
    EnvTable env;
    std::map<std::string, int> temRegMap;

};




#endif // MIPSGENE