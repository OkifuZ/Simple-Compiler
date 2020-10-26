#ifndef ERRHAND_H_INCLUDED
#define ERRHAND_H_INCLUDED

#include <string>
#include <vector>


#define _CAT_VAR 0
#define _CAT_CONST 1
#define _CAT_FUNC 2

#define _TYPE_ERROR -2
#define _TYPE_VOID -1
#define _TYPE_INT 0
#define _TYPE_CHAR 1


class SymTableEntry {
public:
	SymTableEntry(std::string name_, int cate, int type_, int layer_):
		category(cate), name(name_), type(type_), layer(layer_) {}

	virtual ~SymTableEntry() = default;

	virtual std::string getName() { return name; }
	virtual int getCATE() { return category; }
	virtual int getTYPE() { return type; }
	virtual int getLAYER() { return layer; }
	virtual int getINDEX() { return INDEX_TABLE; }
	virtual void setINDEX(int i) { INDEX_TABLE = i; }

protected:
	int category = 0;
	// 0 -> var, 1 -> const, 2 -> func
	int type = 0;
	// -1 -> void, 0 -> int, 1 -> char, also records the return type of a function
	int layer = -1;
	// -1 -> invalid, the outest layer is denoted as 0
	std::string name;
	int INDEX_TABLE = -1;

};


class ArraySymEntry : public SymTableEntry {
public:
	ArraySymEntry(std::string name, int cate, int type, int layer, int size):
		SymTableEntry(name, cate, type, layer), 
		dim(1), firstDimSize(size), secDimSize(-1) {}
	ArraySymEntry(std::string name, int cate, int type, int layer, int size1, int size2) :
		SymTableEntry(name, cate, type, layer),
		dim(2), firstDimSize(size1), secDimSize(size2) {}

	int getDim() { return dim; }
	int getFIRST_SIZE() { return firstDimSize; }
	int getSECOND_SIZE() { return secDimSize; }

private:
	int dim = 0;
	int firstDimSize = -1;
	int secDimSize = -1;
};

class ScalerSymEntry : public SymTableEntry {
public:
	ScalerSymEntry(std::string name, int cate, int type, int layer):
		SymTableEntry(name, cate, type, layer) {}

};

class FormalVarSymEntry : public ScalerSymEntry {
public:
	FormalVarSymEntry(std::string name, int cate, int type, int layer, int parFuncPos_):
		ScalerSymEntry(name, cate, type, layer), parFuncPos(parFuncPos_) {}

	int getPAR_FUNC_POS() { return parFuncPos; }
	void setPAR_FUNC_POS(int pos) { parFuncPos = pos; }

private:
	int parFuncPos = -1;
};

class FuncSymEntry : public SymTableEntry {
public:
	FuncSymEntry(std::string name, int cate, int type, int layer, int argnum=0) :
		SymTableEntry(name, cate, type, layer), argNum(argnum) {}

	void addParaType(int type) { paraTypeList.push_back(type); }

	int getARGNUM() { return argNum; }
	std::vector<int> 
		getParaTypeList() { return paraTypeList; }

	void setARGNUM(int n) { argNum = n; }

private:
	int argNum = -1;
	std::vector<int> paraTypeList;

};


class SymbolTable {
public:
	SymbolTable() {}

	void insertSymbolEntry(SymTableEntry* sym);

	void popSym(int layer);

	bool duplicateName(std::string name, int layer);

	int getTypeByName(std::string name);
	SymTableEntry* getSymByName(std::string name);

	void printSymTable(std::ostream& os);

	// void popSymbol() { symTable.pop_back(); }
	// SymTableEntry* topSymbol() { return symTable.back(); }

private:
	std::vector<SymTableEntry*> symTable;
	
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