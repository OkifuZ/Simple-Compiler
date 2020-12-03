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

void MipsGenerator::printMipsCode(ostream& os) {
    for (auto line : this->mipsCodeList) {
        switch(line->op) {
            case MIPS_INS::DEBUG:
                os << "\n# " << line->x << "\n";
                break;
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
                    os << "div" << " " << line->z << ", " << line->x << ", " << line->immediate << "\n"; 
                }
                else {
                    os << "div" << " " << line->z << ", " << line->x << ", " << line->y << "\n";
                }
                break;
            case MIPS_INS::J: // x = label
                os << "j" << " " << line->x << "\n";
                break;
            case MIPS_INS::JAL:
                os << "jal" << " " << line->x << "\n";
                break;
            case MIPS_INS::JR:
                os << "jr" << " " << line->x << "\n";
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
                os <<line->x << ": " << "\n";
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