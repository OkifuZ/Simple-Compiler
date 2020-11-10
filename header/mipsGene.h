#pragma once

#ifndef MIPSGENE
#define MIPSGENE

#include <string>
#include <vector>
#include "InterCode.h"
#include "tool.h"
#include "parser.h"
#include "errHand.h"

enum class MIPS_INS{
    ERROR=-1, ADDU=0, ADDIU, SUBU, LI, LA, MOVE, 
    MULT, MUL, DIVU, MFLO, MFHI,
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

    std::string getEmptyReg();

private:
    std::vector<InterCodeEntry*> interCodeList;
    std::vector<MipsEntry*> mipsCodeList;
    std::vector<std::string> stringList;
    EnvTable env;

};




#endif // MIPSGENE