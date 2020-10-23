#include "errHand.h"
#include <vector>
#include <string>
#include <tuple>

using namespace std;


TYPE_SYM_CLASS SymTableEntry::getClass() {
	auto tup = make_tuple(category, type, dim, isFormalPar);
	if (tup == make_tuple(CAT_VAR, TYPE_INT, 0, false)) {
		return TYPE_SYM_CLASS::VAR_INT;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_CHAR, 0, false)) {
		return TYPE_SYM_CLASS::VAR_CHAR;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_INT, 0, true)) {
		return TYPE_SYM_CLASS::PARA_INT;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_CHAR, 0, true)) {
		return TYPE_SYM_CLASS::PARA_CHAR;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_CHAR, 1, false)) {
		return TYPE_SYM_CLASS::INT_ARR_DIM1;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_CHAR, 1, false)) {
		return TYPE_SYM_CLASS::CHAR_ARR_DIM1;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_INT, 2, false)) {
		return TYPE_SYM_CLASS::INT_ARR_DIM2;
	}
	else if (tup == make_tuple(CAT_VAR, TYPE_CHAR, 2, false)) {
		return TYPE_SYM_CLASS::CHAR_ARR_DIM2;
	}
	else if (tup == make_tuple(CAT_CONST, TYPE_INT, 0, false)) {
		return TYPE_SYM_CLASS::CON_INT;
	}
	else if (tup == make_tuple(CAT_CONST, TYPE_CHAR, 0, false)) {
		return TYPE_SYM_CLASS::CON_CHAR;
	}
	else if (tup == make_tuple(CAT_FUNC, TYPE_VOID, 0, false)) {
		return TYPE_SYM_CLASS::VOID_FUNC;
	}
	else if (tup == make_tuple(CAT_FUNC, TYPE_INT, 0, 0)) {
		return TYPE_SYM_CLASS::INT_FUNC;
	}
	else if (tup == make_tuple(CAT_FUNC, TYPE_CHAR, 0, 0)) {
		return TYPE_SYM_CLASS::CHAR_FUNC;
	}
	return TYPE_SYM_CLASS::INVALID;
}

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
		string s = symTable[i].name;
		int la = symTable[i].layer;
		if (layer == la && strSame(s, name)) {
			return true;
		}
	}
	return false;
}