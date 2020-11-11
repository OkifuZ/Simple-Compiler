#include <string>
#include <vector>
#include <iostream>
#include "../header/mipsGene.h"
#include "../header/tool.h"
#include "../header/InterCode.h"
#include "../header/globalRec.h"


using namespace std;

string MipsGenerator::getRegister(string name) {
    if (name[0] == '#') { // temVar
        string reg = regPool.getTemRegByName(name);
        if (reg != "") { // already exists a temreg
            return reg;
        }
        else if (temRegMap.find(name) != temRegMap.end()) { // exists in memory
            int offset = temRegMap[name];
            reg = regPool.getEmptyTemReg();
            if (reg != "") { // exists a free temreg, load value

            }
            else { // no free temreg, 
                reg = regPool.graspTemReg();
            }
        }
        else { // a new temvar
            reg = regPool.getEmptyTemReg();
            if (reg != "") { // allocate a new reg
                return reg;
            }
            else { // no free temreg

            }

        }

    }
}

void MipsGenerator::GeneMipsCode() {
    addEntry(new MipsEntry(MIPS_INS::DATASEG, "", "", "", 0, false));
    for (int i = 0; i < stringList.size(); i++) {
        addEntry(new MipsEntry(MIPS_INS::STRINGSEG, " ", "s__"+int2str(i), stringList[i], 0, false));
    }
    addEntry(new MipsEntry(MIPS_INS::STRINGSEG, "", "newline", "\n", 0, false));
    addEntry(new MipsEntry(MIPS_INS::TEXTSEG, "", "", "", 0, false));
    string curFuncName = "";
    for (int i = 0; i < interCodeList.size(); i++) {
        InterCodeEntry* inter = interCodeList[i];
        string zr, xr, yr;
        switch(inter->op) {
            case INT_OP::FUNC:
                addEntry(new MipsEntry(MIPS_INS::LABEL, "", inter->x->name, "", 0, false));
                curFuncName = inter->x->name;
                break;
            case INT_OP::ASSIGN:
                zr = getRegister(inter->z->name);
                if (inter->x->isCon) {
                    int imm = str2int(inter->x->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, "$0", "", imm, true));
                }
                else {
                    xr = getRegister(inter->x->name);
                    addEntry(new MipsEntry(MIPS_INS::MOVE, zr, xr, "", 0, false));
                }
                break;
            case INT_OP::ADD:
                zr = getRegister(inter->z->name);
                xr = getRegister(inter->x->name);
                if (inter->y->isCon) {
                    int imm = str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, xr, "", imm, true));
                }
                else {
                    yr = getRegister(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDU, zr, xr, yr, 0, false));
                }
                break;
            case INT_OP::SUB:
                zr = getRegister(inter->z->name);
                xr = getRegister(inter->x->name);
                if (inter->y->isCon) {
                    int imm = str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, "", imm, true));
                }
                else {
                    yr = getRegister(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, yr, 0, false));
                }
                break;
            case INT_OP::MULT:
                zr = getRegister(inter->z->name);
                xr = getRegister(inter->x->name);
                if (inter->y->isCon) {
                    int imm = str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, "", imm, true));
                }
                else {
                    yr = getRegister(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, yr, 0, false));
                }
                break;
            case INT_OP::DIV:
                zr = getRegister(inter->z->name);
                xr = getRegister(inter->x->name);
                if (inter->y->isCon) {
                    int imm = str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, "", imm, true));
                }
                else {
                    yr = getRegister(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, yr, 0, false));
                }
                break;
            case INT_OP::PRINT: // print "str", var
                if (inter->x->isValid) {
                    string strName = "s__"+int2str(getStringIndex(inter->x->name));
                    addEntry(new MipsEntry(MIPS_INS::LA, "$a0", strName, "", 0, true));
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 4, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                }
                if (inter->y->isValid) { // y may be const int, const char, vat int, var char
                    if (inter->y->isCon) {
                        int imm = str2int(inter->y->name);
                        if (inter->y->type == _TYPE_INT) {
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 1, true));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, "$a0", "$0", "", imm, true));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                        else if (inter->y->type == _TYPE_CHAR){
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 11, true));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, "$a0", "$0", "", imm, true));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                    }
                    else {
                        yr = getRegister(inter->y->name);
                        if (inter->y->type == _TYPE_INT) {
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 1, true));
                            addEntry(new MipsEntry(MIPS_INS::MOVE, "$a0", yr, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                        else if (inter->y->type == _TYPE_CHAR) {
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 11, true));
                            addEntry(new MipsEntry(MIPS_INS::MOVE, "$a0", yr, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                    }
                }
                addEntry(new MipsEntry(MIPS_INS::LA, "$a0", "newline", "", 0, true));
                addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 4, true));
                addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                break;
            case INT_OP::SCAN:
                zr = getRegister(inter->z->name);
                if (inter->z->type == _TYPE_CHAR) {
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 12, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                    addEntry(new MipsEntry(MIPS_INS::MOVE, zr, "$v0", "", 0, false));
                }
                else if (inter->z->type == _TYPE_INT) {
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 5, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                    addEntry(new MipsEntry(MIPS_INS::MOVE, zr, "$v0", "", 0, false));
                }
                break;
            case INT_OP::EXIT:
                addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 12, true));
                addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                break;
            default:
                break;
        }
    }

}

void MipsGenerator::printMipsCode(ostream& os) {
    for (auto line : this->mipsCodeList) {
        switch(line->op) {
            case MIPS_INS::ADDU:
                os << "addu" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                break;
            case MIPS_INS::ADDIU:
                os << "addiu" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n";
                break;
            case MIPS_INS::SUBU:
                if (line->hasImmediate) {
                    os << "subu" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "subu" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                }
                break;
            case MIPS_INS::MOVE:
                os << "move" << " " << line->z << ", " << line->x << "\n";
                break;
            case MIPS_INS::MUL:
                if (line->hasImmediate) {
                    os << "mul" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "mul" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                }
                break;
            case MIPS_INS::DIVU:
                if (line->hasImmediate) {
                    os << "divu" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "divu" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                }
                break;
            case MIPS_INS::J: // x = label
                os << "j" << " " << line->x << "\n";
                break;
            case MIPS_INS::LA: // z = tar, x = label, y = reg, immediate = offset
                if (line->hasImmediate && line->y!="") { // la $1, label+10($2)
                    os << "la" << " " << line->z << ", " << line->x << "+" << line->immediate << "(" << line->y << ")" << "\n";
                }
                else if (line->hasImmediate && line->y == "") { // la $1, label+10 
                    os << "la" << " " << line->z << ", " << line->x << "+" << line->immediate << "\n";
                }
                break;
            case MIPS_INS::LABEL: // x = label
                os << line->x << ": " << "\n";
                break;
            case MIPS_INS::LI:
                os << "li" << " " << line->z << ", " << line->x << "\n";
                break;
            case MIPS_INS::LW: // z = tar, x = reg, y = label, immediate = offset
                if (line->x!= "" && line->hasImmediate && line->y!="") { // lw $1, label+10($2)
                    os << "lw" << " " << line->z << ", " << line->y << "+" << line->immediate << "(" << line->x <<")" << "\n";
                }
                else if (line->x!= "" && line->hasImmediate && line->y=="") { // lw $1, 10($2)
                    os << "lw" << " " << line->z << ", " << line->immediate << "(" << line->x <<")" << "\n";
                }
                else if (line->x== "" && line->hasImmediate && line->y!="") { // lw $1, label+10 
                    os << "lw" << " " << line->z << ", " << line->y << "+" << line->immediate << "\n";
                }
                break;
            case MIPS_INS::SW: // z = tar, x = reg, y = label, immediate = offset
                if (line->x!= "" && line->hasImmediate && line->y!="") { // sw $1, label+10($2)
                    os << "sw" << " " << line->z << ", " << line->y << "+" << line->immediate << "(" << line->x <<")" << "\n";
                }
                else if (line->x!= "" && line->hasImmediate && line->y=="") { // sw $1, 10($2)
                    os << "sw" << " " << line->z << ", " << line->immediate << "(" << line->x <<")" << "\n";
                }
                else if (line->x== "" && line->hasImmediate && line->y!="") { // sw $1, label+10 
                    os << "sw" << " " << line->z << ", " << line->y << "+" << line->immediate << "\n";
                }
                break;
            case MIPS_INS::STRINGSEG: // x = name, y = string
                os << line->x << ": " << ".asciiz" << " \"" << line->y << "\"\n";
                break;
            case MIPS_INS::DATASEG:
                os << ".data\n";
                break;
            case MIPS_INS::TEXTSEG:
                os << ".text\n";
                break; 
            case MIPS_INS::SYSCALL:
                os << "syscall\n";
                break;
            default:
                break;


        }
    }
}