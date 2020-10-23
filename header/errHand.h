#ifndef ERRHAND_H_INCLUDED
#define ERRHAND_H_INCLUDED

#include <string>
#include <vector>


#define CAT_VAR 0
#define CAT_CONST 1
#define CAT_FUNC 2

#define TYPE_ERROR -2
#define TYPE_VOID -1
#define TYPE_INT 0
#define TYPE_CHAR 1


enum class TYPE_SYM_CLASS {
	INVALID = -1, 
	VAR_INT = 0, VAR_CHAR, 
	INT_ARR_DIM1, INT_ARR_DIM2, 
	CHAR_ARR_DIM1, CHAR_ARR_DIM2,
	CON_INT, CON_CHAR,
	INT_FUNC, CHAR_FUNC, VOID_FUNC,
	PARA_INT, PARA_CHAR
};

class SymTableEntry {
public:
	int category = 0;
	// 0 -> var, 1 -> const, 2 -> func
	int type = 0; 
	// -1 -> void, 0 -> int, 1 -> char, also records the return type of a function
	int dim = 0;
	// scaler or func -> 0, type[] -> 1, type[][] -> 2
	int layer = -1;
	// -1 -> invalid, the outest layer is denoted as 0
	std::string name;

	bool isFormalPar = false;
	//int parFuncPos = -1;
	int arguNum = -1;
	int firstDimSize = -1;
	int secDimSize = -1;
	// if is formal parameter, funcPos >= 0
	//int arrayPos = -1;
	// if is array(dim >= 1), arrayPos >= 0
	//int funcPos = -1;
	//// if is function, funcPos >= 0

	SymTableEntry(std::string name, int cate, int type_, int layer_, int dim_=0, int size1=-1, int size2=-1) :
		category(cate), type(type_), layer(layer_), dim(dim_), firstDimSize(size1), secDimSize(size2) {}
	
	SymTableEntry(std::string name, int cate, int type_, int layer_, bool isFormal, int argunum/*, int parFuncPos_*/) :
		category(cate), type(type_), layer(layer_), isFormalPar(isFormal), arguNum(argunum)/*, parFuncPos(parFuncPos_)*/ {}



	TYPE_SYM_CLASS getClass();

};


//class ArrayTableEntry{
//public:
//	int firstDimSize = -1;
//	int secDimSize = -1;
//};


class SymbolTable {
private:
	std::vector<SymTableEntry> symTable;
	//std::vector<ArrayTableEntry> arrayTable;
	
public:
	SymbolTable() {}

	void insertSymbol(std::string name, int cate, int type, int layer, int dim=0, int size1=-1, int size2=-1) {
		symTable.push_back(SymTableEntry(name, cate, type, layer, dim, size1, size2));
	}
	void insertSymbol(std::string name, int cate, int type, int layer, bool isFormal, int argunum=-1/*, int funcPos*/) {
		symTable.push_back(SymTableEntry(name, cate, type, layer, isFormal, argunum/*, funcPos*/));
	}

	void popSymbol() {
		symTable.pop_back();
	}

	SymTableEntry topSymbol() {
		return symTable.back();
	}

	bool duplicateName(std::string name, int layer);

	int idenTYPE(std::string name, int layer);


};

class ErrorMessage {
public:
	int line;
	char iden;
	std::string message;
	ErrorMessage(int line_, char iden_, std::string mess) : 
		line(line_), iden(iden_), message(mess) {}
};


#endif // ERRHAND_H_INCLUDED