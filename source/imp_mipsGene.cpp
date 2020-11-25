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

int MipsGenerator::varInARegister(string name) {
    for (int i = 0; i < 4; i++) {
        if (aRegister[i].varName == name) {
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
                temRegister[ind].setVar(name);
                if (load) {
                    loadValue(name, reg);
                }
                return reg;
            }
        }
    }
    else { // local var or global var or formal arg
        SymTableEntry* symEnt = getSymByName(name);
        FormalVarSymEntry* forEnt = dynamic_cast<FormalVarSymEntry*>(symEnt);
        if (forEnt != nullptr) { // formal arg
            int i = varInARegister(name);
            if (i != -1) {  // 0th,1st,2nd,3rd arg
                return aRegister[i].regName;
            }
            else { // 4th ... arg
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
                    temRegister[ind].setVar(name);
                    if (load) {
                        loadValue(name, reg);
                    }
                    return reg;
                }
            }
        }
        else { // local or global var
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
                    temRegister[ind].setVar(name);
                    if (load) {
                        loadValue(name, reg);
                    }
                    return reg;
                }
            }
        }
    }
}

string MipsGenerator::graspTemReg() {
    int i;
    for (i = prevPos; i < 10; i++) {
        if (usingInCurInter.find("$t"+int2str(i)) != usingInCurInter.end()) {
            continue;
        }
        else {
            string reg = "$t"+int2str(i);
            usingInCurInter.insert(reg);
            prevPos = (i+1)%9;
            storeBack(reg, false, false);
            return reg;
        }
    }
    return "";
}

void MipsGenerator::freeReg(string regName) { // freeReg, won't store back
    if (regName[1] == 't') {
        int index = str2int(regName.substr(2, 1));
        // string name = temRegister[index].varName;
        // if (temVarOffsetMap.find(name) != temVarOffsetMap.end()) {
        //     temVarOffsetMap.erase(name);
        // }
        temRegister[index].setFree();
    }
    else if (regName[1] == 's') {
        int index = str2int(regName.substr(2, 1));
        globalRegister[index].setFree();
    }
}

void MipsGenerator::storeBack(string regName, bool allocate, bool fake) { // has mem or no mem
    
    if (fake) {
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
        topOffset += 4;
    }
    else if (allocate) {
        addEntry(new MipsEntry(MIPS_INS::SW, regName, "$sp", "", 0, true)); // store $fp of caller
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
        topOffset += 4/*$ra*/;
    }
    else if (regName[1] == 't') { // tem reg
        int ind = str2int(regName.substr(2, 1));
        string varName = temRegister[ind].varName;
        if (varName[0] == '#'  && temRegister[ind].isBusyT) { // tem Var
            if (temVarOffsetMap.find(varName) != temVarOffsetMap.end()) { // tem var has memory
                int offset = temVarOffsetMap[varName] - 4;
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
            }
            else { // tem var has no memory, allocate and store it
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$sp", "", 0, true));
                addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
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
                int offset = sym->offset - 4;
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
            }
        }
        temRegister[ind].setFree();
    }
    else if (regName[1] == 's') {// global reg, only contains local var
        int ind = str2int(regName.substr(2, 1));
        string varName = globalRegister[ind].varName;
        SymTableEntry* sym = getSymByName(varName);
        int offset = sym->offset - 4;
        addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
        globalRegister[ind].setFree();
    }
    
}

void MipsGenerator::loadValue(string name, string reg) {
    if (name[0] == '#') { // tem var
        if (temVarOffsetMap.find(name) != temVarOffsetMap.end()) {
            int offset = temVarOffsetMap[name] - 4;
            addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", -offset, true));
        }
        else {
            cout << "hey, you can't load a temVar has no memory allocated!" << endl;
        }
    }
    else {
        SymTableEntry* sym = getSymByName(name);
        if (sym!= nullptr) { 
            if (sym->isGlobal) { // global var
                int offset = sym->offset; // dataseg goes upward
                addEntry(new MipsEntry(MIPS_INS::LW, reg, "", "globalData", offset, true));
            }
            else { // local var or formal arg
                FormalVarSymEntry* forEnt = dynamic_cast<FormalVarSymEntry*>(sym);
                if (forEnt != nullptr) { // formal arg
                    int offset = forEnt->argId * 4 + 4; // TODO
                    addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", offset, true));
                }
                else {
                    int offset = sym->offset - 4; // stack goes downward
                    addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", -offset, true));
                }
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
        ArraySymEntry* arrSymEntry = dynamic_cast<ArraySymEntry*>(symEntry);
        if (arrSymEntry != nullptr) {
            int dim = arrSymEntry->getDim();
            arrSymEntry->offset = offset;
            if (dim == 1) {
                offset += 4 * arrSymEntry->getFIRST_SIZE();
            }
            else if (dim == 2) {
                offset += 4 * arrSymEntry->getFIRST_SIZE() * arrSymEntry->getSECOND_SIZE();
            }
        }
        else if (symEntry->getCATE() != _CAT_FUNC && symEntry->getCATE() != _CAT_CONST) {
            FormalVarSymEntry* argSym = dynamic_cast<FormalVarSymEntry*>(symEntry);
            if (argSym == nullptr) { // is not formal arguement
                string name = symEntry->getName();
                if (k < 8) {
                    globalRegister[k].setFree();
                    globalRegister[k].setVar(name);
                }
                offset += 4;
                symEntry->offset = topOffset + offset;
                k++;
            }
        }
    }
    if (symTab->symTable.size() > 0 && offset > 0) {
        topOffset += offset;
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", offset, true));
    }
}

/*
    | argList     | 
    | $ra         | 
    $fp/$sp0        
    | caller reg  |  
    | callee var  |
    | callee tem  |
    $sp cur
*/

void MipsGenerator::storeGloRegOfCaller(SymbolTable* symTab) {
    int k = 0;
    for (int i = 0; i < symTab->symTable.size(); i++) {
        SymTableEntry* symEntry = symTab->symTable[i];
        ArraySymEntry* arrSymEntry = dynamic_cast<ArraySymEntry*>(symEntry);
        if (arrSymEntry != nullptr) {
            // array has no globalRegister
            // pass
        }
        else if (symEntry->getCATE() != _CAT_FUNC && symEntry->getCATE() != _CAT_CONST) {
            // local scaler
            FormalVarSymEntry* argSym = dynamic_cast<FormalVarSymEntry*>(symEntry);
            if (argSym == nullptr) { // is not formal arguement
                string name = symEntry->getName();
                if (k < 8) { // callee will use this register 
                    /* 
                        caller or caller's caller may have taken this GloReg, 
                        callee has to store it (dobby has to stop it!) 
                    */
                    storeBack(globalRegister[k].regName, true, false);
                }
                k++;
            }
        }
    }
    topOffset +=  k*4/*GloReg*/;
}

void MipsGenerator::restoreGloRegOfCaller(SymbolTable* symTab) {
    int k = 0;
    for (int i = 0; i < symTab->symTable.size(); i++) {
        SymTableEntry* symEntry = symTab->symTable[i];
        ArraySymEntry* arrSymEntry = dynamic_cast<ArraySymEntry*>(symEntry);
        if (arrSymEntry != nullptr) {
            // array has no globalRegister
            // pass
        }
        else if (symEntry->getCATE() != _CAT_FUNC && symEntry->getCATE() != _CAT_CONST) {
            // local scaler
            FormalVarSymEntry* argSym = dynamic_cast<FormalVarSymEntry*>(symEntry);
            if (argSym == nullptr) { // is not formal arguement
                string name = symEntry->getName();
                if (k < 8) { // callee has used this register
                    addEntry(new MipsEntry(MIPS_INS::LW, globalRegister[k].regName, "$fp", "", -(k+1)*4, true)); // k+1 for $ra
                }
                k++;
            }
        }
    }
}

void MipsGenerator::storeCallerTemReg() {
    for (int i = 0; i < 10; i++) {
        storeBack("$t" + int2str(i), true, false);
    }
}

void MipsGenerator::restoreCallerTemReg() {
    for (int i = 0; i < 10; i++) {
        addEntry(new MipsEntry(MIPS_INS::LW, "$t"+int2str(9 - i), "$sp", "", (i+1)*4, true));
    }
    addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", 9*4, true));
}

void MipsGenerator::storeCallerAReg() {
    for (int i = 0; i < 4; i++) {
        storeBack("$a" + int2str(i), true, false);
    }
}

void MipsGenerator::restoreCallerAReg() {
    for (int i = 0; i < 4; i++) {
        addEntry(new MipsEntry(MIPS_INS::LW, "$a"+int2str(3 - i), "$sp", "", (i+1)*4, true));
    }
    addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", 4*4, true));
}

void MipsGenerator::pushReg(string reg, bool fake) {
    if (fake) {
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
        topOffset += 4;
    }
    else {
        addEntry(new MipsEntry(MIPS_INS::SW, reg, "$sp", "", 0, true)); 
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
        topOffset += 4;
    }
}

void MipsGenerator::popReg(string reg) {
    addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", 4, true));
    addEntry(new MipsEntry(MIPS_INS::LW, reg, "$sp", "", 0, false));
}

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

    static bool firstFuncMeet = false;
    // code
    for (int i = 0; i < interCodeList.size(); i++) {
        InterCodeEntry* inter = interCodeList[i];
        #ifdef PRINT_ERROR_MESSAGE
            InterCodeEntry* line = inter;
            stringstream os;
            std::vector<std::string> INT_OP_STR = { "ADD", "SUB", "MULT", "DIV", "ASSIGN", 
                                                    "SCAN", "PRINT", "J", "EXIT", "FUNC", 
                                                    "ENDFUNC", "ARRINI",
                                                    "BLE", "BLT", "BGE", "BGT", "BNE", "BEQ", "LABEL",
                                                    "JAL", "BEFCALL", "ENDCALL", "PARA", "PUSH"};
            os << INT_OP_STR[static_cast<int>(line->op)] << " ";
            if (line->op == INT_OP::FUNC) {
                if (!firstFuncMeet) { // first function meet, must jump to main
                    // jump to main
                    firstFuncMeet = true;
                    addEntry(new MipsEntry(MIPS_INS::J, "", "main", "", 0, false));
                }
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
        #endif
        string zr="", xr="", yr="";
        switch(inter->op) {
            case INT_OP::FUNC: {
                addEntry(new MipsEntry(MIPS_INS::LABEL, "", inter->x->name, "", 0, false)); // add label
                curFuncName = inter->x->name;
                SymbolTable* funcTab = env.getTableByFuncName(curFuncName);
                if (inter->x->name != "main") { // main has nothing related with $ra, caller's GloReg
                    storeBack("$ra", true, false);
                    storeBack("$fp", true, false);
                    storeGloRegOfCaller(funcTab); // store callee used global register which caller has taken up
                }
                assignGloReg2LocVar(funcTab); // allocate memory space of callee's local variable
                break;
            }
            case INT_OP::ENDFUNC: {
                if (inter->x->name != "main") {
                    curFuncName = inter->x->name;
                    SymbolTable* funcTab = env.getTableByFuncName(curFuncName);
                    restoreGloRegOfCaller(funcTab);
                    addEntry(new MipsEntry(MIPS_INS::LW, "$ra", "$fp", "", 4, true)); // restore $ra
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$sp", "$fp", "", 0, false)); // set $fp to $sp
                    addEntry(new MipsEntry(MIPS_INS::LW, "$fp", "$fp", "", 0, true));
                    addEntry(new MipsEntry(MIPS_INS::JR, "", "$ra", "", 0, false)); // jump to $ra
                }
                topOffset = 0;
                break;
            }
            case INT_OP::BEFCALL: {
                // store back temReg
                storeCallerTemReg();
                storeCallerAReg();
                storeBack("$fp", true, false);
                break;
            }
            case INT_OP::PUSH: {
                string valueArgName = line->x->name;
                int num = str2int(line->y->name);
                string temReg = getRegister(valueArgName, true);
                if (num <= 3) {
                    // a0 ... a3
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$a"+int2str(num), temReg, "", 0, false));
                    storeBack("", false, true); // take place but nothing stored
                }
                else {
                    storeBack(temReg, true, false);
                }
                break;
            }
            case INT_OP::ENDCALL: {
                // TODO
                string funcName = line->x->name;
                SymTableEntry* tabEnt = env.root->getSymByName(funcName);
                FuncSymEntry* funcTabEnt = dynamic_cast<FuncSymEntry*>(tabEnt);
                if (funcTabEnt != nullptr) {
                    
                    // pop arglist
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", funcTabEnt->getARGNUM()*4, true));
                    restoreCallerAReg();
                    restoreCallerTemReg();
                }
                else {
                    cout << "endcall a non func" << endl;
                }
                
                break;
            }
            case INT_OP::ASSIGN: {
                InterCodeEntry_array* interArr = dynamic_cast<InterCodeEntry_array*>(inter);
                if (interArr != nullptr) {
                    SymTableEntry* sym = getSymByName(interArr->arrInRight ? interArr->rv->name : interArr->z->name);
                    ArraySymEntry* symArr = dynamic_cast<ArraySymEntry*>(sym);
                    #ifdef PRINT_ERROR_MESSAGE
                        if (symArr == nullptr) {
                            cout << "assign a scaler as an array";
                        }
                    #endif
                    int offset = symArr->offset;
                    int real_offset = -1;
                    string real_offset_reg;
                    // calculate offset
                    if (symArr->getDim() == 1) {
                        int con_x;
                        bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                        bool imm_x = isConstX | inter->x->isCon;
                        if (imm_x) {
                            real_offset = (isConstX ? con_x : str2int(interArr->x->name)) * 4 + offset;
                        }
                        else {
                            string temVar = "#temvar_array_assign";
                            string reg_i = getRegister(temVar, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, temVar, getRegister(interArr->x->name, true), "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_i, reg_i, "", 4, true));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_i, reg_i, "", offset, true));
                            real_offset_reg = reg_i;
                        }
                    }
                    else { // 2
                        int con_x;
                        bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                        bool imm_x = isConstX | inter->x->isCon;
                        int con_y;
                        bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                        bool imm_y = isConstY | inter->y->isCon;
                        if (imm_x && imm_y) {
                            int x_value = isConstX ? con_x : str2int(interArr->x->name);
                            int y_value = isConstY ? con_y : str2int(interArr->y->name);
                            real_offset = (x_value * symArr->getFIRST_SIZE() + y_value) * 4 + offset;
                        }
                        else if (imm_x && !imm_y) {
                            int x_value = isConstX ? con_x : str2int(interArr->x->name);
                            int base_off = x_value * symArr->getFIRST_SIZE(); // i*first_size
                            string temVar = "#temvar_array_assign";
                            string reg_j = getRegister(temVar, false);
                            string temReg = getRegister(interArr->y->name, true);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, reg_j, temReg, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_j, reg_j, "", base_off, true)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_j, reg_j, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_j, reg_j, "", offset, true));
                            real_offset_reg = reg_j;
                        }
                        else if (!imm_x && imm_y) {
                            int y_value = isConstY ? con_y : str2int(interArr->y->name);
                            string temVar = "#temvar_array_assign";
                            string reg_i = getRegister(temVar, false);
                            string temReg = getRegister(interArr->x->name, true);
                            int sec_off = y_value;
                            addEntry(new MipsEntry(MIPS_INS::MOVE, reg_i, temReg, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_i, reg_i, "", symArr->getFIRST_SIZE(), true)); // i * first_size
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_i, reg_i, "", sec_off, true)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_i, reg_i, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_i, reg_i, "", offset, true));
                            real_offset_reg = reg_i;
                        }
                        else {
                            string reg_i = getRegister(interArr->x->name, true);
                            string reg_j = getRegister(interArr->y->name, true);
                            string temVar = "#temvar_array_assign";
                            string temReg = getRegister(temVar, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, temReg, reg_i, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::MUL, temReg, temReg, "", symArr->getFIRST_SIZE(), true)); // i * first_size
                            addEntry(new MipsEntry(MIPS_INS::ADDU, temReg, temReg, reg_j, 0, false)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, temReg, temReg, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, temReg, temReg, "", offset, true));
                            real_offset_reg = temReg;
                        }
                    }
                    // LW OR SW
                    if (interArr->arrInRight) { // tem = arr[i][j]
                        string temVar = "#temvar_array_assign_ano";
                        string temReg = getRegister(temVar, false);
                        if (real_offset == -1) { // offset is reg
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, real_offset_reg, "globalData", 0, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::SUBU, real_offset_reg, "$fp", real_offset_reg, 0, false));
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, real_offset_reg, "", 0, true));
                            }
                                freeReg(real_offset_reg);
                        }
                        else { // offset is con
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, "", "globalData", real_offset, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, "$fp", "", -real_offset, true));
                            }
                        }
                        zr = getRegister(inter->z->name, false);
                        addEntry(new MipsEntry(MIPS_INS::MOVE, zr, temReg, "", 0, false));
                        freeReg(temReg);
                    }
                    else { // arr[i][j] = tem(rv)
                        int con_rv;
                        bool isConstRV = interArr->rv->isCon ? false : checkIsConst(interArr->rv->name, &con_rv);
                        bool imm_rv = isConstRV | interArr->rv->isCon;
                        string rvr;
                        if (imm_rv) {
                            int rv_int = -1;
                            rv_int = isConstRV ? con_rv : str2int(interArr->rv->name);
                            rvr = getRegister("#temvar_array_assign_ano", false);
                            addEntry(new MipsEntry(MIPS_INS::LI, rvr, "", "", rv_int, true));
                        }
                        else {
                            rvr = getRegister(interArr->rv->name, true);
                        }
                        if (real_offset == -1) { // offset is reg
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, real_offset_reg, "globalData", 0, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::SUBU, real_offset_reg, "$fp", real_offset_reg, 0, false));
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, real_offset_reg, "", 0, true));
                            }
                            freeReg(real_offset_reg);
                        }
                        else { // offset is con
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, "", "globalData", real_offset, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, "$fp", "", -real_offset, true));
                            }
                        }
                        freeReg(rvr);
                    }
                }
                else { // scaler
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
                }
                break;
            }
            case INT_OP::ARRINI: {
                InterCodeEntry_arrDec* interArr = dynamic_cast<InterCodeEntry_arrDec*>(inter);
                string name = interArr->z->name;
                SymTableEntry* sym = getSymByName(name);
                vector<int> iniList = (interArr->iniList);
                int offset = sym->offset;
                for (int i = 0; i < iniList.size(); i++) {
                    string temReg = getRegister("#temvar_arrini", false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", iniList[i], true));
                    if (sym->isGlobal) {
                        addEntry(new MipsEntry(MIPS_INS::SW, temReg, "", "globalData", offset + i * 4, true));
                    }
                    else {
                        addEntry(new MipsEntry(MIPS_INS::SW, temReg, "$fp", "", -(offset + i * 4), true));
                    }
                    freeReg(temReg);
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
                    string temReg = getRegister("#temp_use_inDiv", false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true)); // tem = 5
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, temReg, yr, 0, false)); // z = tem / x
                    freeReg(temReg);
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
                addEntry(new MipsEntry(MIPS_INS::J, "", inter->x->name, "", 0, false));
                break;
            }
            case INT_OP::LABEL: {
                addEntry(new MipsEntry(MIPS_INS::LABEL, "", inter->x->name, "", 0, false));
                break;
            }
            case INT_OP::BEQ: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, temReg, "", yv, true));
                    freeReg(temReg);
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, xr, yr, 0, false));
                }

                break;
            }
            case INT_OP::BGE: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::BGT: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::BLT: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::BLE: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::BNE: {
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true);
                    yr = getRegister(inter->y->name, true);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, xr, yr, 0, false));
                }
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
            case MIPS_INS::BEQ:
                if (line->hasImmediate) {
                    os << "beq"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "beq"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
                break;
            case MIPS_INS::BNE:
                if (line->hasImmediate) {
                    os << "bne"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "bne"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
                break;
            case MIPS_INS::BLT:
                if (line->hasImmediate) {
                    os << "blt"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "blt"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
                break;
            case MIPS_INS::BLE:
                if (line->hasImmediate) {
                    os << "ble"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "ble"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
                break;
            case MIPS_INS::BGT:
                if (line->hasImmediate) {
                    os << "bgt"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "bgt"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
                break;
            case MIPS_INS::BGE:
                if (line->hasImmediate) {
                    os << "bge"  << " " << line->x << ", " << line->immediate << ", " << line->z << "\n";
                }
                else {
                    os << "bge"  << " " << line->x << ", " << line->y << ", " << line->z << "\n";
                }
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
            case MIPS_INS::JR:
                os << "jr" << " " << line->x << "\n";
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