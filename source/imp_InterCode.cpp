﻿#include <string>
#include <vector>
#include <iostream>
#include "../header/InterCode.h"
#include "../header/tool.h"

using namespace std;


void Intermediate::addInterCode(INT_OP op, string z, int z_type,  
                                           string x, int x_type, bool x_isCon, 
                                           string y, int y_type, bool y_isCon) {
    InterOprand* z_ = new InterOprand(z, z_type, false);
    InterOprand* x_ = new InterOprand(x, x_type, x_isCon);
    InterOprand* y_ = new InterOprand(y, y_type, y_isCon);
    InterCodeEntry* item = new InterCodeEntry(op, x_, y_, z_);
    interCodeList.push_back(item);
}



void Intermediate::printInterCode(ostream& os) {
    for (auto line : this->interCodeList) {
        os << this->INT_OP_STR[static_cast<int>(line->op)] << " ";
        if (line->z->isValid) {
            os << line->z->name << " ";
        }
        if (line->x->isValid) {
            if (line->x->isCon && line->x->type == _TYPE_CHAR) {
                os << "\'" <<line->x->getConstChar() << "\' ";
            }
            else if (line->x->isCon && line->x->type == _TYPE_STR) {
                os << "\"" <<line->x->name << "\" ";
            }
            else {
                os << line->x->name << " ";
            }
        }
        if (line->y->isValid) {
            if (line->y->isCon && line->y->type == _TYPE_CHAR) {
                os << "\'" <<line->y->getConstChar() << "\' ";
            }
            else if (line->y->isCon && line->y->type == _TYPE_STR) {
                os << "\"" <<line->y->name << "\" ";
            }
            else {
                os << line->y->name << " ";
            }
        }
        os << endl;
    }
}

string Intermediate::nextTempVar() {
    temCount++;
    return "#temVar" + int2str(this->temCount);
}