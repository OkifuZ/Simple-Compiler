#include <string>
#include <vector>
#include <iostream>
#include "../header/mipsGene.h"
#include "../header/tool.h"
#include "../header/InterCode.h"
#include "../header/globalRec.h"


using namespace std;

void MipsGenerator::GeneMipsCode() {
    addEntry(new MipsEntry(MIPS_INS::DATASEG, "", "", "", 0, false));
    for (int i = 0; i < stringList.size(); i++) {
        addEntry(new MipsEntry(MIPS_INS::STRINGSEG, " ", "s__"+int2str(i), stringList[i], 0, false));
    }
    addEntry(new MipsEntry(MIPS_INS::STRINGSEG, "", "newline", "\n", 0, false));
    addEntry(new MipsEntry(MIPS_INS::TEXTSEG, "", "", "", 0, false));

    for (int i = 0; i < interCodeList.size(); i++) {
        InterCodeEntry* inter = interCodeList[i];
        switch(inter->op) {
            case INT_OP::ADD:
                
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
            default:
                break;


        }
    }
}