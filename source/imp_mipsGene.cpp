#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "../header/mipsGene.h"
#include "../header/tool.h"
#include "../header/InterCode.h"
#include "../header/globalRec.h"
#include "../header/register.h"


using namespace std;

std::string MipsGenerator::getNameByReg(std::string reg) {
    int ind = str2int(reg.substr(2, 1));
    if (reg[1] == 't') {
        return temRegister[ind].varName;
    }
    else if (reg[1] == 's') {
        return globalRegister[ind].varName;
    }
    return "";
}

int MipsGenerator::varInTemRegister(string name) {
    for (int i = 0; i < 10; i++) {
        if (temRegister[i].varName == name) {
            return i;
        }
    }
    return -1;
}

int MipsGenerator::varInGloRegister(string name) {
    for (int i = 0; i < 8; i++) {
        if (globalRegister[i].varName == name) {
            return i;
        }
    }
    return -1;
}

string MipsGenerator::getRegister(string name, bool load) {
    if (name[0] == '#') {
        int i = varInTemRegister(name); 
        if (i != -1) { // varName exists in temReg
            return temRegister[i].regName;
        }
        else { // varName not in temReg
            string reg = getEmptyTemReg();
            if (reg != "") { // has empty temReg
                int ind = str2int(reg.substr(2, 1));
                temRegister[ind].setVar(name);
                if (load) {
                    loadValue(name, reg);
                }
                return reg;
            }
            else { // no empty temReg
                reg = graspTemReg();
                int ind = str2int(reg.substr(2, 1));
                storeBack(reg);
                temRegister[ind].setVar(name);
                if (load) {
                    loadValue(name, reg);
                }
                return reg;
            }
        }
    }
    else { // local var or global var
        int i = varInGloRegister(name);
        if (i != -1) {  // local var in gloReg
            return globalRegister[i].regName;
        }
        else { // local var not in gloReg, i.e. globalReg is full(pre assigned) 
            int i = varInTemRegister(name);
            if (i != -1) {
                return temRegister[i].regName;
            }
            string reg = getEmptyTemReg();
            if (reg != "") { // has empty temReg
                int ind = str2int(reg.substr(2, 1));
                temRegister[ind].setVar(name);
                if (load) {
                    loadValue(name, reg);
                }
                return reg;
            }
            else { // no empty temReg
                reg = graspTemReg();
                int ind = str2int(reg.substr(2, 1));
                storeBack(reg);
                temRegister[ind].setVar(name);
                if (load) {
                    loadValue(name, reg);
                }
                return reg;
            }
        }
    }
}

string MipsGenerator::graspTemReg() {
    for (int i = prevPos; i < 10; i++) {
        if (usingInCurInter.find("$t"+int2str(i)) != usingInCurInter.end()) {
            continue;
        }
        else {
            string reg = "$t"+int2str(i);
            usingInCurInter.insert(reg);
            prevPos = (i+1)%9;
            return reg;
        }
    }
    return "";
}

void MipsGenerator::storeBack(string regName) { // has mem or no mem
    int ind = str2int(regName.substr(2, 1));
    if (regName[1] == 't') { // tem reg
        string varName = temRegister[ind].varName;
        if (varName[0] == '#'  && temRegister[ind].isBusyT) { // tem Var
            if (temVarOffsetMap.find(varName) != temVarOffsetMap.end()) { // tem var has memory
                int offset = -temVarOffsetMap[varName];
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", offset, true));
            }
            else { // tem var has no memory, allocate and store it
                addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$sp", "", 0, true));
                topOffset += 4;
                temVarOffsetMap.insert(make_pair(varName, topOffset));
            }
        } 
        else if (temRegister[ind].isBusyL) { // local var or globa; var
            SymTableEntry* sym = getSymByName(varName);
            if (sym->isGlobal) { // global var
                int offset = sym->offset;
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "", "globalData", offset, true));
            }
            else { // local var
                int offset = -sym->offset;
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", offset, true));
            }
        }
        temRegister[ind].setFree();
    }
    else if (regName[1] == 's') {// global reg
        string varName = globalRegister[ind].varName;
        SymTableEntry* sym = getSymByName(varName);
        int offset = -sym->offset;
        addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", offset, true));
        globalRegister[ind].setFree();
    }
}

void MipsGenerator::loadValue(string name, string reg) {
    if (name[0] == '#') { // tem var
        if (temVarOffsetMap.find(name) != temVarOffsetMap.end()) {
            int offset = -temVarOffsetMap[name];
            addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", offset, true));
        }
    }
    else {
        SymTableEntry* sym = getSymByName(name);
        if (sym!= nullptr) { // global var
            if (sym->isGlobal) {
                int offset = sym->offset; // dataseg goes upward
                addEntry(new MipsEntry(MIPS_INS::LW, reg, "", "globalData", offset, true));
            }
            else { // local var
                int offset = -(sym->offset); // stack goes downward
                addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", offset, true));
            }
        }
    }
}

SymTableEntry* MipsGenerator::getSymByName(std::string name) {
    SymbolTable* funcSym = env.getTableByFuncName(curFuncName);
    SymTableEntry* sym = funcSym->getSymByName(name);
    if (sym == nullptr) {
        sym = env.root->getSymByName(name);
    }
    return sym;
}

bool MipsGenerator::checkIsConst(std::string name, int* value) {
    SymTableEntry* sym = getSymByName(name);
    if (sym == nullptr) {
        return false;
    }
    if (sym->category == _CAT_CONST) {
        if (dynamic_cast<ScalerSymEntry*>(sym) != nullptr) {
            *value = dynamic_cast<ScalerSymEntry*>(sym)->value;
        }
        return true;
    }
    return false;

}

void MipsGenerator::assignGloReg2LocVar(SymbolTable* symTab) {
    /*for (int i = 0; i < temRegister.size(); i++) {
        if (temRegister[i].isBusyL || temRegister[i].isBusyT) {
            temRegister[i].setFree();
        }
    }*/
    int k = 0;
    int offset = 0;
    for (int i = 0; i < symTab->symTable.size(); i++) {
        SymTableEntry* symEntry = symTab->symTable[i];
        if (symEntry->getCATE() != _CAT_FUNC && symEntry->getCATE() != _CAT_CONST) {
            string name = symEntry->getName();
            if (k < 8) {
                globalRegister[k].setVar(name);
            }
            offset = k*4;
            symEntry->offset = offset;
            k++;
        }
    }
    topOffset += offset;
    if (symTab->symTable.size() > 0) {
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", topOffset, true));
    }
    // TODO SUB SP
}
// TODO CONST
void MipsGenerator::GeneMipsCode() {
    addEntry(new MipsEntry(MIPS_INS::DATASEG, "", "", "", 0, false));
    // global vars
    SymbolTable* globalTab = env.getTableByFuncName("global");
    int totalGlobalOffset = globalTab->calculateOffset();
    addEntry(new MipsEntry(MIPS_INS::SPACE, "", "globalData", "", totalGlobalOffset, true));
    // string con
    for (int i = 0; i < stringList.size(); i++) {
        addEntry(new MipsEntry(MIPS_INS::STRINGSEG, " ", "s__"+int2str(i), stringList[i], 0, false));
    }
    addEntry(new MipsEntry(MIPS_INS::STRINGSEG, "", "newline_", "\\n", 0, false));
    // text
    addEntry(new MipsEntry(MIPS_INS::TEXTSEG, "", "", "", 0, false));
    // assign $fp to $sp
    addEntry(new MipsEntry(MIPS_INS::MOVE, "$fp", "$sp", "", 0, false));

    // code
    for (int i = 0; i < interCodeList.size(); i++) {
        InterCodeEntry* inter = interCodeList[i];
        if (true) {
            InterCodeEntry* line = inter;
            stringstream os;
            std::vector<std::string> INT_OP_STR = { "ADD", "SUB", "MULT", "DIV", "ASSIGN", "SCAN", "PRINT", "J", "EXIT", "FUNC", "ENDFUNC" };
            os << INT_OP_STR[static_cast<int>(line->op)] << " ";
            if (line->op == INT_OP::FUNC) {
                os << ": " << line->x->name << "\n";
            }
            else {
                if (line->z->isValid) {
                    os << line->z->name << " ";
                }
                if (line->x->isValid) {
                    if (line->x->isCon && line->x->type == _TYPE_CHAR) {
                        os << "\'" << line->x->getConstChar() << "\' ";
                    }
                    else if (line->x->isCon && line->x->type == _TYPE_STR) {
                        os << "\"" << line->x->name << "\" ";
                    }
                    else {
                        os << line->x->name << " ";
                    }
                }
                if (line->y->isValid) {
                    if (line->y->isCon && line->y->type == _TYPE_CHAR) {
                        os << "\'" << line->y->getConstChar() << "\' ";
                    }
                    else if (line->y->isCon && line->y->type == _TYPE_STR) {
                        os << "\"" << line->y->name << "\" ";
                    }
                    else {
                        os << line->y->name << " ";
                    }
                }
            }
            string s = os.str();
            addEntry(new MipsEntry(MIPS_INS::DEBUG, "", s, "", 0, false));
        }
        string zr="", xr="", yr="";
        switch(inter->op) {
            case INT_OP::FUNC: {
                // global assign statement
                if (inter->x->name == "main") {
                    // jump to main
                    addEntry(new MipsEntry(MIPS_INS::J, "", "main", "", 0, false));
                }
                addEntry(new MipsEntry(MIPS_INS::LABEL, "", inter->x->name, "", 0, false));
                curFuncName = inter->x->name;
                SymbolTable* funcTab = env.getTableByFuncName(curFuncName);
                assignGloReg2LocVar(funcTab);
                break;
            }
            case INT_OP::ASSIGN: {
                zr = getRegister(inter->z->name, false);
                if (inter->x->isCon) {
                    int imm = str2int(inter->x->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, "$0", "", imm, true));
                }
                else {
                    int imm = 0;
                    if (checkIsConst(inter->x->name, &imm)) {
                            addEntry(new MipsEntry(MIPS_INS::LI, zr, "", "", imm, true));
                    }
                    else {
                        xr = getRegister(inter->x->name, true);
                        addEntry(new MipsEntry(MIPS_INS::MOVE, zr, xr, "", 0, false));
                    }
                }
                break;
            }
            case INT_OP::ADD: {
                zr = getRegister(inter->z->name, true);
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::LI, zr, "", "", xv + yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::ADDU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::SUB: {
                zr = getRegister(inter->z->name, true);
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::LI, zr, "", "", xv - yv, true));
                }
                else if (isImm_x && !isImm_y) { // z = 5 - y
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, yr, "", xv, true)); // z = y - 5
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, "$0", zr, 0, false)); // z = -z = 5 - y
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::MULT: {
                zr = getRegister(inter->z->name, true);
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::LI, zr, "", "", xv * yv, true));
                }
                else if (isImm_x && !isImm_y) { // z = 5 * y
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, yr, "", xv, true)); // z = y * 5
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::DIV: {
                zr = getRegister(inter->z->name, true);
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::LI, zr, "", "", xv / yv, true));
                }
                else if (isImm_x && !isImm_y) { // z = 5 * y
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    string tem = getRegister("#temp_use_inDiv", false);
                    addEntry(new MipsEntry(MIPS_INS::LI, tem, "", "", xv, true)); // tem = 5
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, tem, yr, 0, false)); // z = tem / x
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::PRINT: {// print "str", var
                if (inter->x->isValid) {
                    string strName = "s__" + int2str(getStringIndex(inter->x->name));
                    addEntry(new MipsEntry(MIPS_INS::LA, "$a0", strName, "", 0, true));
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 4, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                }
                if (inter->y->isValid) { // y may be const int, const char, vat int, var char
                    int con_y;
                    bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                    if (inter->y->isCon || isConstY) {
                        int imm = isConstY ? con_y : str2int(inter->y->name);
                        if (inter->y->type == _TYPE_INT) {
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 1, true));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, "$a0", "$0", "", imm, true));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                        else if (inter->y->type == _TYPE_CHAR) {
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 11, true));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, "$a0", "$0", "", imm, true));
                            addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                        }
                    }
                    else {
                        yr = getRegister(inter->y->name, true);
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
                addEntry(new MipsEntry(MIPS_INS::LA, "$a0", "newline_", "", 0, true));
                addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 4, true));
                addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                break;
            }
            case INT_OP::SCAN: {
                zr = getRegister(inter->z->name, false);
                if (inter->z->type == _TYPE_CHAR) {
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 12, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                    addEntry(new MipsEntry(MIPS_INS::MOVE, zr, "$v0", "", 0, false));
                    // read \n
                    /*addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 12, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));  */                  
                }
                else if (inter->z->type == _TYPE_INT) {
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 5, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                    addEntry(new MipsEntry(MIPS_INS::MOVE, zr, "$v0", "", 0, false));
                }
                break;
            }
            case INT_OP::J: {
                // string funcNameStr = inter->x->name;
                // SymbolTable* funcSymTab = env.getTableByFuncName(funcNameStr);
                // if (funcSymTab != nullptr) {
                //        temVarOffsetMap.clear();
                // }
                break;
            }
            case INT_OP::EXIT: {
                addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 10, true));
                addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                break;
            }
            default:
                break;
        }
        usingInCurInter.clear();
    }

}

void MipsGenerator::printMipsCode(ostream& os) {
    for (auto line : this->mipsCodeList) {
        switch(line->op) {
            case MIPS_INS::DEBUG:
                os << "\n# " << line->x << "\n";
                break;
            case MIPS_INS::ADDU:
                os << "add" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                break;
            case MIPS_INS::ADDIU:
                os << "addi" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n";
                break;
            case MIPS_INS::SUBU:
                if (line->hasImmediate) {
                    os << "sub" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "sub" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
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
                    os << "div" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "div" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
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
                os << "\n" <<line->x << ": " << "\n";
                break;
            case MIPS_INS::LI:
                os << "li" << " " << line->z << ", " << line->immediate << "\n";
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
                os << "\n.text\n";
                break; 
            case MIPS_INS::SPACE: // x = name, imm = bytes
                os << line->x << ": " << ".space" << " " << line->immediate << "\n";
                break;
            case MIPS_INS::SYSCALL:
                os << "syscall\n";
                break;
            default:
                break;


        }
    }
}