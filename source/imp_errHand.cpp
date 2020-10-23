#include <vector>
#include <string>
#include <tuple>
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

bool SymbolTable::duplicateName(string name, int layer) {
	for (int i = symTable.size() - 1; i >= 0; i--) {
		string s = symTable[i]->getName();
		int la = symTable[i]->getLAYER();
		if (layer == la && strSame(s, name)) {
			return true;
		}
	}
	return false;
}

SymTableEntry* SymbolTable::getSymByName(std::string name) {
	for (int i = symTable.size() - 1; i >= 0; i--) {
		string s = symTable[i]->getName();
		if (strSame(s, name)) {
			symTable[i];
		}
	}
	return nullptr;
}

int SymbolTable::idenTYPE(string name, int layer) {
	for (int i = symTable.size() - 1; i >= 0; i--) {
		string s = symTable[i]->getName();
		int la = symTable[i]->getLAYER();
		if (layer >= la && strSame(s, name)) {
			return symTable[i]->getTYPE();
		}
	}
	return -2;
}