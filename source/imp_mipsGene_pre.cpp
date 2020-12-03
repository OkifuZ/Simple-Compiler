#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "../header/mipsGene.h"
#include "../header/tool.h"
#include "../header/InterCode.h"
#include "../header/globalRec.h"
#include "../header/register.h"

using namespace std;

void MipsGenerator::preProcessing() {

    for (int i = 0; i < interCodeList.size(); i++) {
        InterCodeEntry* inter = interCodeList[i];
        if (inter->op == INT_OP::FUNC) {
            SymbolTable* funcTab = env.getTableByFuncName(inter->x->name);
            int j;
            for (j = i + 1; j < interCodeList.size(); j++) {
                inter = interCodeList[j];
                
                if (inter->z->name[0] == '#' && funcTab->getTemSymByName(inter->z->name) == nullptr) {
                    funcTab->temVarTable.push_back(new TemVarSymEntry(inter->z->name, inter->z->type));
                }
                if (inter->x->name[0] == '#' && funcTab->getTemSymByName(inter->x->name) == nullptr) {
                    funcTab->temVarTable.push_back(new TemVarSymEntry(inter->x->name, inter->x->type));
                }
                if (inter->y->name[0] == '#' && funcTab->getTemSymByName(inter->y->name) == nullptr) {
                    funcTab->temVarTable.push_back(new TemVarSymEntry(inter->y->name, inter->y->type));
                }
                if (inter->op == INT_OP::ENDFUNC) {
                    break;
                }
                
            }
            i = j;
        }
    }

}