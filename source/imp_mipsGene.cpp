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
                                                    "JAL", "BEFCALL", "ENDCALL", /*"PARA",*/ "PUSH",
                                                    "RETURN"};
            os << INT_OP_STR[static_cast<int>(line->op)] << " ";
            if (line->op == INT_OP::FUNC) {
                if (!firstFuncMeet) { // first function meet, must jump to main
                    // jump to main
                    firstFuncMeet = true;
                    // only this way!!!
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
                    
                    // fresh topOffset
                    //topOffsetQue.push_back(topOffset);
                    topOffset = 0;
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$fp", "$sp", "", 0, false));
                    pushReg("$ra");
                    // storeBack("$ra", true, false);
                    pushAllGloRegOfCaller(funcTab); // store callee used global register which caller has taken up
                    // set a0 ... a3 as arguments
                    string funcName = line->x->name;
                    SymTableEntry* tabEnt = env.root->getSymByName(funcName);
                    FuncSymEntry* funcTabEnt = dynamic_cast<FuncSymEntry*>(tabEnt);
                    for (int i = 0; i < funcTabEnt->formalArgNameList.size(); i++) {
                        if  (i <= 3) {
                            aRegister[i].setVar(funcTabEnt->formalArgNameList[i]);
                        }
                    }
                }
                else { // main
                    // assign $fp to $sp
                    topOffset = 0;
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$fp", "$sp", "", 0, false));
                }
                assignGloReg2LocVar(funcTab); // allocate memory space of callee's local variable
                break;
            }
            case INT_OP::ENDFUNC: {
                if (inter->x->name != "main") {
                    curFuncName = inter->x->name;
                    SymbolTable* funcTab = env.getTableByFuncName(curFuncName);
                    restoreGloRegOfCaller(funcTab);
                    addEntry(new MipsEntry(MIPS_INS::LW, "$ra", "$fp", "", 0, true)); // restore $ra, no need to change sp and topoffset
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$sp", "$fp", "", 0, false)); // set $fp to $sp
                    addEntry(new MipsEntry(MIPS_INS::JR, "", "$ra", "", 0, false)); // jump to $ra
                    //topOffset = topOffsetQue.back();
                    //topOffsetQue.pop_back();
                    freeAllTemReg(); // !
                    freeAllGloReg();
                    freeAllAreg();

                }
                break;
            }
            case INT_OP::RETURN: {
                if (inter->z->name != "main") {
                    curFuncName = inter->z->name;
                    SymbolTable* funcTab = env.getTableByFuncName(curFuncName);
                    if (inter->x->name != "") { // non void return
                        int con_x = 0;
                        bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                        bool isImm_x = isConstX | inter->x->isCon;
                        if (isImm_x) {
                            int xv = isConstX ? con_x : str2int(inter->x->name);
                            addEntry(new MipsEntry(MIPS_INS::LI, "$v1", "", "", xv, true));
                        }
                        else {
                            string retReg = getRegister(inter->x->name, true, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, "$v1", retReg, "", 0, false));
                        }
                    }
                    restoreGloRegOfCaller(funcTab);
                    addEntry(new MipsEntry(MIPS_INS::LW, "$ra", "$fp", "", 0, true)); // restore $ra, no need to change sp and topoffset
                    addEntry(new MipsEntry(MIPS_INS::MOVE, "$sp", "$fp", "", 0, false)); // set $fp to $sp
                    addEntry(new MipsEntry(MIPS_INS::JR, "", "$ra", "", 0, false)); // jump to $ra
                    topOffset = topOffsetQue.back();
                    // no need when return
                    // topOffsetQue.pop_back();
                    //freeAllTemReg(); // !
                }
                else {
                    addEntry(new MipsEntry(MIPS_INS::LI, "$v0", "", "", 10, true));
                    addEntry(new MipsEntry(MIPS_INS::SYSCALL, "", "", "", 0, false));
                }
                break;
            }
            case INT_OP::BEFCALL: {
                // store back temReg
                
                storeCallerTemReg();
                storeCallerAReg();
                pushReg("$fp");
                //storeBack("$fp", true, false);
                break;
            }
            case INT_OP::PUSH: {
                int con_x = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isImm_x = isConstX | inter->x->isCon;
                int num = str2int(line->y->name);
                if (isImm_x) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    if (num <= 3) {
                        // a0 ... a3
                        addEntry(new MipsEntry(MIPS_INS::LI, "$a"+int2str(num), "", "", xv, true));
                        pushReg("$a"+int2str(num), true);
                        //storeBack("", false, true); // take place but nothing stored
                    }
                    else {
                        addEntry(new MipsEntry(MIPS_INS::LI, "$k0", "", "", xv, true));
                        pushReg("$k0");
                        //pushReg(temReg);
                        //storeBack(temReg, true, false);
                    }
                }
                else {
                    string valueArgName = line->x->name;
                    string temReg = getRegister(valueArgName, true, true);
                    if (num <= 3) {
                        // a0 ... a3
                        addEntry(new MipsEntry(MIPS_INS::MOVE, "$a"+int2str(num), temReg, "", 0, false));
                        pushReg("$a"+int2str(num), true);
                        //storeBack("", false, true); // take place but nothing stored
                    }
                    else {
                        pushReg(temReg);
                        //storeBack(temReg, true, false);
                    }
                }
                break;
            }
            case INT_OP::ENDCALL: {
                // TODO
                string funcName = line->x->name;
                SymTableEntry* tabEnt = env.root->getSymByName(funcName);
                FuncSymEntry* funcTabEnt = dynamic_cast<FuncSymEntry*>(tabEnt);
                if (funcTabEnt != nullptr) {
                    if (line->y->name != "") { // ret value
                        addEntry(new MipsEntry(MIPS_INS::DEBUG, "", "load ret value", "", 0, false));
                        string retReg = getRegister(line->y->name, false, false);
                        addEntry(new MipsEntry(MIPS_INS::MOVE, retReg, "$v1", "", 0, false));
                    }
                    // pop arglist
                    addEntry(new MipsEntry(MIPS_INS::DEBUG, "", "pop arg list", "", 0, false));
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, "$sp", "$sp", "", funcTabEnt->getARGNUM()*4, true));
                    // restore fp
                    addEntry(new MipsEntry(MIPS_INS::DEBUG, "", "restore fp", "", 0, false));
                    popReg("$fp");
                    // addEntry(new MipsEntry(MIPS_INS::DEBUG, "", "restore caller's AReg", "", 0, false));
                    // seems no need to do that
                    // restoreCallerAReg();
                    // seems no need to do that too
                    // restoreCallerTemReg();
                }
                else {
                    cout << "endcall a non func" << endl;
                }
                
                break;
            }
            case INT_OP::JAL: {
                // freeAllTemReg();
                addEntry(new MipsEntry(MIPS_INS::JAL, "", line->x->name, "", 0, false));
                freeAllTemReg(); // ! this line is definetly meaningless???
                freeAllAreg();
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
                            string temVar = "#&temvar_array_assign";
                            string reg_i = getRegister(temVar, false, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, reg_i, getRegister(interArr->x->name, true, false), "", 0, false));
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
                            real_offset = (x_value * symArr->getSECOND_SIZE() + y_value) * 4 + offset;
                        }
                        else if (imm_x && !imm_y) {
                            int x_value = isConstX ? con_x : str2int(interArr->x->name);
                            int base_off = x_value * symArr->getSECOND_SIZE(); // i*first_size
                            string temVar = "#&temvar_array_assign";
                            string reg_j = getRegister(temVar, false, false);
                            string temReg = getRegister(interArr->y->name, true, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, reg_j, temReg, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_j, reg_j, "", base_off, true)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_j, reg_j, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_j, reg_j, "", offset, true));
                            real_offset_reg = reg_j;
                        }
                        else if (!imm_x && imm_y) {
                            int y_value = isConstY ? con_y : str2int(interArr->y->name);
                            string temVar = "#&temvar_array_assign";
                            string reg_i = getRegister(temVar, false, false);
                            string temReg = getRegister(interArr->x->name, true, false);
                            int sec_off = y_value;
                            addEntry(new MipsEntry(MIPS_INS::MOVE, reg_i, temReg, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_i, reg_i, "", symArr->getSECOND_SIZE(), true)); // i * first_size
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_i, reg_i, "", sec_off, true)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, reg_i, reg_i, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, reg_i, reg_i, "", offset, true));
                            real_offset_reg = reg_i;
                        }
                        else {
                            string reg_i = getRegister(interArr->x->name, true, false);
                            string reg_j = getRegister(interArr->y->name, true, false);
                            string temVar = "#&temvar_array_assign";
                            string temReg = getRegister(temVar, false, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, temReg, reg_i, "", 0, false));
                            addEntry(new MipsEntry(MIPS_INS::MUL, temReg, temReg, "", symArr->getSECOND_SIZE(), true)); // i * first_size
                            addEntry(new MipsEntry(MIPS_INS::ADDU, temReg, temReg, reg_j, 0, false)); // i*first_size + j
                            addEntry(new MipsEntry(MIPS_INS::MUL, temReg, temReg, "", 4, true)); // (i*first_size + j)*4
                            addEntry(new MipsEntry(MIPS_INS::ADDIU, temReg, temReg, "", offset, true));
                            real_offset_reg = temReg;
                        }
                    }
                    // LW OR SW
                    if (interArr->arrInRight) { // tem = arr[i][j]
                        string temVar = "#&temvar_array_assign_ano";
                        string temReg = getRegister(temVar, false, false);
                        if (real_offset == -1) { // offset is reg
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, real_offset_reg, "globalData", 0, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::SUBU, real_offset_reg, real_offset_reg, "", 4, true));
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
                                addEntry(new MipsEntry(MIPS_INS::LW, temReg, "$fp", "", -(real_offset-4), true));
                            }
                        }
                        zr = getRegister(inter->z->name, false, false);
                        addEntry(new MipsEntry(MIPS_INS::MOVE, zr, temReg, "", 0, false));
                        freeReg(temReg);
                    }
                    else { // arr[i][j] = rv
                        int con_rv;
                        bool isConstRV = interArr->rv->isCon ? false : checkIsConst(interArr->rv->name, &con_rv);
                        bool imm_rv = isConstRV | interArr->rv->isCon;
                        string rvr;
                        bool isTem = false;
                        if (imm_rv) {
                            int rv_int = -1;
                            rv_int = isConstRV ? con_rv : str2int(interArr->rv->name);
                            rvr = getRegister("#&temvar_array_assign_ano", false, false);
                            isTem = true;
                            addEntry(new MipsEntry(MIPS_INS::LI, rvr, "", "", rv_int, true));
                        }
                        else {
                            rvr = getRegister(interArr->rv->name, true, false);
                        }
                        if (real_offset == -1) { // offset is reg
                            if (symArr->isGlobal) {
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, real_offset_reg, "globalData", 0, true));
                            }
                            else {
                                addEntry(new MipsEntry(MIPS_INS::SUBU, real_offset_reg, real_offset_reg, "", 4, true));
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
                                addEntry(new MipsEntry(MIPS_INS::SW, rvr, "$fp", "", -(real_offset-4), true));
                            }
                        }
                        if (isTem) {
                            freeReg(rvr);
                        }
                    }
                }
                else { // scaler
                    zr = getRegister(inter->z->name, false, false);
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
                            xr = getRegister(inter->x->name, true, false);
                            addEntry(new MipsEntry(MIPS_INS::MOVE, zr, xr, "", 0, false));
                        }
                    }
                    SymTableEntry* symEnt = getSymByName(inter->z->name);
                    if (symEnt != nullptr && symEnt->isGlobal/* && curFuncName == "global" */) {
                        storeBack(zr);
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
                    string temReg = getRegister("#&temvar_arrini", false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", iniList[i], true));
                    if (sym->isGlobal) {
                        addEntry(new MipsEntry(MIPS_INS::SW, temReg, "", "globalData", offset + i * 4, true));
                    }
                    else {
                        addEntry(new MipsEntry(MIPS_INS::SW, temReg, "$fp", "", -(offset + i * 4 - 4), true));
                    }
                    freeReg(temReg);
                }
                break;
            }
            case INT_OP::ADD: {
                // if you don't load, and xr or yr is the same as zr, then things goes wrong
                // any solutions?
                // let me see, if 
                if (idenSame(inter->z->name, inter->x->name) || idenSame(inter->z->name, inter->y->name)) {
                    zr = getRegister(inter->z->name, true, false);
                }
                else {
                    zr = getRegister(inter->z->name, false, false);
                }
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
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::ADDIU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::ADDU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::SUB: {
                if (idenSame(inter->z->name, inter->x->name) || idenSame(inter->z->name, inter->y->name)) {
                    zr = getRegister(inter->z->name, true, false);
                }
                else {
                    zr = getRegister(inter->z->name, false, false);
                }
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
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, yr, "", xv, true)); // z = y - 5
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, "$0", zr, 0, false)); // z = -z = 5 - y
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::SUBU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::MULT: {
                if (idenSame(inter->z->name, inter->x->name) || idenSame(inter->z->name, inter->y->name)) {
                    zr = getRegister(inter->z->name, true, false);
                }
                else {
                    zr = getRegister(inter->z->name, false, false);
                }
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
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, yr, "", xv, true)); // z = y * 5
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::MUL, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::DIV: {
                if (idenSame(inter->z->name, inter->x->name) || idenSame(inter->z->name, inter->y->name)) {
                    zr = getRegister(inter->z->name, true, false);
                }
                else {
                    zr = getRegister(inter->z->name, false, false);
                }
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
                    yr = getRegister(inter->y->name, true, false);
                    string temReg = getRegister("#temp_use_inDiv", false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true)); // tem = 5
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, temReg, yr, 0, false)); // z = tem / x
                    freeReg(temReg);
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::DIVU, zr, xr, yr, 0, false));
                }
                break;
            }
            case INT_OP::PRINT: {// print "str", var
                storeBack("$a0");
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
                        yr = getRegister(inter->y->name, true, false);
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
                // load back $a0
                loadValue(aRegister[0].varName, "$a0");
                // string funcName = line->x->name;
                // SymTableEntry* tabEnt = env.root->getSymByName(funcName);
                // FuncSymEntry* funcTabEnt = dynamic_cast<FuncSymEntry*>(tabEnt);
                // addEntry(new MipsEntry(MIPS_INS::LW, "$a0", "$fp", "", funcTabEnt->getARGNUM() * 4, true));
                break;
            }
            case INT_OP::SCAN: {
                zr = getRegister(inter->z->name, false, false);
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
                storeCallerAReg();
                storeCallerTemReg();
                addEntry(new MipsEntry(MIPS_INS::J, "", inter->x->name, "", 0, false));
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::LABEL: {
                storeCallerAReg();
                storeCallerTemReg();
                addEntry(new MipsEntry(MIPS_INS::LABEL, "", inter->x->name, "", 0, false));
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BEQ: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, temReg, "", yv, true));
                    freeReg(temReg);
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BEQ, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BGE: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BGT: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BLT: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BGT, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BLT, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BLE: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BGE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BLE, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
                break;
            }
            case INT_OP::BNE: {
                storeCallerAReg();
                storeCallerTemReg();
                int con_x = 0, con_y = 0;
                bool isConstX = inter->x->isCon ? false : checkIsConst(inter->x->name, &con_x);
                bool isConstY = inter->y->isCon ? false : checkIsConst(inter->y->name, &con_y);
                bool isImm_x = isConstX | inter->x->isCon;
                bool isImm_y = isConstY | inter->y->isCon;
                if (isImm_x && isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    string temVar = "#condition";
                    string temReg = getRegister(temVar, false, false);
                    addEntry(new MipsEntry(MIPS_INS::LI, temReg, "", "", xv, true));
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, temReg, "", yv, true));
                }
                else if (isImm_x && !isImm_y) {
                    int xv = isConstX ? con_x : str2int(inter->x->name);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, yr, "", xv, true));
                }
                else if (!isImm_x && isImm_y) {
                    xr = getRegister(inter->x->name, true, false);
                    int yv = isConstY ? con_y : str2int(inter->y->name);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, xr, "", yv, true));
                }
                else {
                    xr = getRegister(inter->x->name, true, false);
                    yr = getRegister(inter->y->name, true, false);
                    addEntry(new MipsEntry(MIPS_INS::BNE, inter->z->name, xr, yr, 0, false));
                }
                freeAllAreg();
                freeAllTemReg();
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
