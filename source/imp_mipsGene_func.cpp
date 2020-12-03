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
        if (idenSame(temRegister[i].varName, name)) {
            return i;
        }
    }
    return -1;
}

int MipsGenerator::varInGloRegister(string name) {
    for (int i = 0; i < 8; i++) {
        if (idenSame(globalRegister[i].varName, name)) {
            return i;
        }
    }
    return -1;
}

int MipsGenerator::varInARegister(string name) {
    for (int i = 0; i < 4; i++) {
        if (idenSame(aRegister[i].varName, name)) {
            return i;
        }
    }
    return -1;
}

string MipsGenerator::getRegister(string name, bool load, bool noStore) {
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
                reg = graspTemReg(noStore);
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
                i = varInTemRegister(name);
                if (i != -1) { // varName exists in temReg
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
                    reg = graspTemReg(noStore);
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
                    reg = graspTemReg(noStore);
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

string MipsGenerator::graspTemReg(bool noStore) {
    int i;
    for (i = prevPos; i < 10; i++) {
        if (usingInCurInter.find("$t"+int2str(i)) != usingInCurInter.end()) {
            continue;
        }
        else {
            string reg = "$t"+int2str(i);
            usingInCurInter.insert(reg);
            prevPos = (i+1)%9;
            if (!noStore) {
                storeBack(reg); 
            }
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

void MipsGenerator::storeBack(string regName, bool noFree) { // has mem or no mem
    
    // if (fake) {
    //     addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
    //     topOffset += 4;
    // }
    // else if (allocate) {
    //     addEntry(new MipsEntry(MIPS_INS::SW, regName, "$sp", "", 0, true)); // store $fp of caller
    //     addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
    //     topOffset += 4/*$ra*/;
    // }
    // else 
    if (regName[1] == 't') { // tem reg
        int ind = str2int(regName.substr(2, 1));
        if (!temRegister[ind].isBusyL && !temRegister[ind].isBusyT && idenSame(temRegister[ind].varName, "")) {
            return;
        }
        string varName = temRegister[ind].varName;
        if (varName[0] == '#'  && temRegister[ind].isBusyT) { // tem Var
            if (varName[1] == '&') {
                return;
            }
            TemVarSymEntry* temEnt = getTemVarSymByName(varName);
            int offset = temEnt->offset - 4;
            addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
            // if (temVarOffsetMap.find(varName) != temVarOffsetMap.end()) { // tem var has memory
            //     int offset = temVarOffsetMap[varName] - 4;
            //     addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
            // }
            // else { // tem var has no memory, allocate and store it
            //     pushReg(regName);
            //     // addEntry(new MipsEntry(MIPS_INS::SW, regName, "$sp", "", 0, true));
            //     // addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", 4, true));
            //     // topOffset += 4;
            //     temVarOffsetMap.insert(make_pair(varName, topOffset));
            // }
        } 
        else if (temRegister[ind].isBusyL) { // local var or globa var or formal arg
            SymTableEntry* sym = getSymByName(varName);
            if (sym->isGlobal) { // global var
                int offset = sym->offset;
                addEntry(new MipsEntry(MIPS_INS::SW, regName, "", "globalData", offset, true));
            }
            else { // local var or formal var
                FormalVarSymEntry* forEnt = dynamic_cast<FormalVarSymEntry*>(sym);
                if (forEnt != nullptr) { // formal arg
                    int offset = (forEnt->getFuncArgNum() - forEnt->argId) * 4; // argNum(n) - argId(0...n-1)
                    addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", offset, true));
                }
                else { // local var
                    int offset = sym->offset - 4;
                    addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
                }
            }
        }
        if (!noFree) {
            temRegister[ind].setFree();
        }
    }
    else if (regName[1] == 's') {// global reg, only contains local var
        int ind = str2int(regName.substr(2, 1));
        if (!globalRegister[ind].isBusyL && !globalRegister[ind].isBusyT && idenSame(globalRegister[ind].varName, "")) {
            return;
        }
        string varName = globalRegister[ind].varName;
        SymTableEntry* sym = getSymByName(varName);
        int offset = sym->offset - 4;
        addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", -offset, true));
        if (!noFree) {
            globalRegister[ind].setFree();
        }
    }
    else if (regName[1] == 'a') {
        int ind = str2int(regName.substr(2, 1));
        if (!aRegister[ind].isBusyL && !aRegister[ind].isBusyT && idenSame(aRegister[ind].varName, "")) {
            return;
        }
        string varName = aRegister[ind].varName;
        SymTableEntry* sym = getSymByName(varName);
        FormalVarSymEntry* fsym = dynamic_cast<FormalVarSymEntry*>(sym);
        if (fsym != nullptr) {
            int offset = (fsym->getFuncArgNum() - fsym->argId) * 4;
            addEntry(new MipsEntry(MIPS_INS::SW, regName, "$fp", "", offset, true));
        }
        else {
            cout << "store back a aReg var which is not formalArg";
        }
        if (!noFree) {
            aRegister[ind].setFree();
        }
    }
}

void MipsGenerator::loadValue(string name, string reg) {
    if (idenSame(name, "")) {
        return;
    }
    if (name[0] == '#') { // tem var
        if (name[1] == '&') {
            return;
        }
        TemVarSymEntry* temEnt = getTemVarSymByName(name);
        int offset = temEnt->offset - 4;
        addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", -offset, true));
        // if (temVarOffsetMap.find(name) != temVarOffsetMap.end()) {
        //     int offset = temVarOffsetMap[name] - 4;
        //     addEntry(new MipsEntry(MIPS_INS::LW, reg, "$fp", "", -offset, true));
        // }
        // else {
        //     cout << "hey, you can't load a temVar has no memory allocated! " << name << endl;
        // }
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
                    int offset = (forEnt->getFuncArgNum() - forEnt->argId) * 4; // argNum(n) - argId(0...n-1)
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

TemVarSymEntry* MipsGenerator::getTemVarSymByName(std::string name) {
    SymbolTable* funcSym = env.getTableByFuncName(curFuncName);
    TemVarSymEntry* sym = funcSym->getTemSymByName(name);
    if (sym == nullptr) {
        cout << "wtf you use global temVar?????" << endl;
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
    int k = 0;
    int offset = 0;
    for (int i = 0; i < symTab->symTable.size(); i++) {
        SymTableEntry* symEntry = symTab->symTable[i];
        ArraySymEntry* arrSymEntry = dynamic_cast<ArraySymEntry*>(symEntry);
        if (arrSymEntry != nullptr) {
            int dim = arrSymEntry->getDim();
            offset += 4;
            arrSymEntry->offset = topOffset + offset;
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
    // temVar
    for (int i = 0; i < symTab->temVarTable.size(); i++) {
        TemVarSymEntry* temSymEnt = symTab->temVarTable[i];
        offset += 4;
        temSymEnt->offset = topOffset + offset;
    }
    if (offset > 0) {
        topOffset += offset;
        addEntry(new MipsEntry(MIPS_INS::SUBU, "$sp", "$sp", "", offset, true));
    }
}

/*
    | tem Reg     |
    | a reg       |
    | old fp      |
    | argList     | 
    | $ra         | <- fp       
    | caller sreg |  
    | callee lvar |
    | callee tvar | <- sp
*/

void MipsGenerator::pushAllGloRegOfCaller(SymbolTable* symTab) {
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
                    pushReg(globalRegister[k].regName);
                    //storeBack(globalRegister[k].regName, true, false);
                }
                k++;
            }
        }
    }
    //topOffset +=  k*4/*GloReg*/;
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
                // TODO
                    addEntry(new MipsEntry(MIPS_INS::LW, globalRegister[k].regName, "$fp", "", -(k+1)*4, true)); // k+1 for $ra
                }
                k++;
            }
        }
    }
}

void MipsGenerator::storeCallerTemReg() {
    for (int i = 0; i < 10; i++) {
        storeBack("$t"+int2str(i), false);
        //pushReg("$t" + int2str(i));
        //storeBack("$t" + int2str(i), true, false);
    }
}

void MipsGenerator::restoreCallerTemReg() {
    // do nothing 
    // for (int i = 0; i < 10; i++) {
        
    //     //popReg("$t"+int2str(9 - i));
    //     //addEntry(new MipsEntry(MIPS_INS::LW, "$t"+int2str(9 - i), "$sp", "", (i+1)*4, true));
    // }
    //addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", 9*4, true));
}

void MipsGenerator::storeCallerAReg() {
    for (int i = 0; i < 4; i++) {
        //pushReg("$a" + int2str(i));
        storeBack("$a" + int2str(i), false);
    }
}

void MipsGenerator::restoreCallerAReg() {
    for (int i = 0; i < 4; i++) {
        popReg("$a"+int2str(3 - i));
        //addEntry(new MipsEntry(MIPS_INS::LW, "$a"+int2str(3 - i), "$sp", "", (i+1)*4, true));
    }
    //addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", 4*4, true));
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
    addEntry(new MipsEntry(MIPS_INS::LW, reg, "$sp", "", 0, true));
    topOffset -= 4;
}
