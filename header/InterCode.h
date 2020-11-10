#pragma once

#ifndef INTERCODE
#define INTERCODE

#include <string>
#include <vector>
#include <iostream>
#include "errHand.h"
#include "tool.h"

enum class INT_OP{ERROR=-1, ADD=0, SUB, MULT, DIV, ASSIGN, SCAN, PRINT, EXIT};

class InterCodeEntry;
class InterOprand;

class Intermediate {
public:

    Intermediate(EnvTable *env_) : env(env_) {}
    
    // z = x op y
    void addInterCode(INT_OP op, std::string z, int z_type,
        std::string x, int x_type, bool isCon1, 
        std::string y, int y_type, bool isCon2); // if not valid, just pass ""

    void printInterCode(std::ostream& os);

    std::string nextTempVar();

    std::vector<InterCodeEntry*> interCodeList;

private:
    int temCount = 0;
    EnvTable* env;
    std::vector<std::string> INT_OP_STR = {"ADD", "SUB", "MULT", "DIV", "ASSIGN", "SCAN", "PRINT", "EXIT"};

};


class InterOprand {
public:
    std::string name;
    int type;
    bool isCon;
    bool isValid;
    bool isImmediate;

    InterOprand(std::string name_, int type_, bool isCon_) : name(name_), type(type_), isCon(isCon_) {
        isValid = !(name == "");
        isImmediate = (name[0] == '#');
    }

    int getConstInt() {
        if (isCon && type == _TYPE_INT) {
            return str2int(name);
        } 
    }

    char getConstChar() {
        if (isCon && type == _TYPE_CHAR) {
            return static_cast<char>(str2int(name));
        }
    }

};


// class InterOprand {
// public:
//     std::string name;
//     SymTableEntry* attr;
//     bool isIntermediate;
//     bool isImmediate;
//     bool isValid;
//     bool isStrCon;

//     InterOprand(std::string name_, SymTableEntry* attr_) : name(name_), attr(attr_) {
//         isValid = !(name == "");
//         if (attr == nullptr) {
//             isIntermediate = name_[0] == '#';
//             isImmediate = isIntstr(name_);
//             isStrCon = !(isIntermediate || isImmediate);
//         } 
//     }

//     int getConstValue() {
//         if (name[0] == '$') {
//             return str2int(name.substr(1, name.size()));
//         }
//         else if (name[0] == '^') {
//             return static_cast<int>(str2char(name.substr(1, name.size())));
//         }
//     }

// };

class InterCodeEntry {
public:
    INT_OP op;
    InterOprand* x;
    InterOprand* y;
    InterOprand* z;

    InterCodeEntry(INT_OP op_, InterOprand* x_, InterOprand* y_, InterOprand* z_) : op(op_), x(x_), y(y_), z(z_) {}
    
    
};









#endif // INTERCODE
