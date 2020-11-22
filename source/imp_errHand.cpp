#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include "../header/errHand.h"

using namespace std;


bool strSame(string a, string b) {
	if (a.size() != b.size()) return false;
	for (int i = 0; i < a.size(); i++) {
		char ac = a[i];
		char bc = b[i];
		if (ac <= 'Z' && ac >= 'A') {
			ac = ac - 'A' + 'a';
		}
		if (bc <= 'Z' && bc >= 'A') {
			bc = bc - 'A' + 'a';
		}
		if (ac != bc) return false;
	}
	return true;
}

bool SymbolTable::duplicateName(string name) {
	for (int i = symTable.size() - 1; i >= 0; i--) {
		string s = symTable[i]->getName();
		if (strSame(s, name)) {
			return true;
		}
	}
	return false;
}

SymTableEntry* SymbolTable::getSymByName(std::string name) {

	for (int i = symTable.size() - 1; i >= 0; i--) {
		string s = symTable[i]->getName();
		if (strSame(s, name)) {
			return symTable[i];
		}
	}
	return nullptr;
}

void SymbolTable::insertSymbolEntry(SymTableEntry* sym) {
	symTable.push_back(sym);
	sym->setINDEX(symTable.size() - 1); // set index(pos) of sym
	sym->isGlobal = (funcName == "global");
}

void SymbolTable::printSymTable(ostream& os) {
	for (auto i : symTable) {
		os << (i->getINDEX()) << " : function->" << funcName <<
			" name->" << (i->getName()) << " type->" << (i->getTYPE()) <<
			" cate->" << (i->getTYPE()) << " offset->" << (i->getOffset()) << endl;
	}
}

int SymbolTable::calculateOffset() { // 0, 4, 8, 12, ...
	int offset = 0;
	for (auto iter: symTable) {
        ArraySymEntry* arrSymEntry = dynamic_cast<ArraySymEntry*>(iter);
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
        else if (iter->category != _CAT_CONST && iter->category != _CAT_FUNC) {
			iter->offset = offset;
			offset += 4;
		}
	}
	return offset;
}