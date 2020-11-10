#ifndef ERRHAND_H_INCLUDED
#define ERRHAND_H_INCLUDED

#include <string>
#include <vector>

#define _INV -7

#define _CAT_VAR 0
#define _CAT_CONST 1
#define _CAT_FUNC 2
#define _CAT_CON 3

#define _TYPE_ERROR -2
#define _TYPE_VOID -1
#define _TYPE_INT 0
#define _TYPE_CHAR 1
#define _TYPE_STR 2


class SymTableEntry {
public:
	SymTableEntry(std::string name_, int cate, int type_):
		category(cate), name(name_), type(type_) {}

	virtual ~SymTableEntry() = default;

	virtual std::string getName() { return name; }
	virtual int getCATE() { return category; }
	virtual int getTYPE() { return type; }
	virtual int getINDEX() { return INDEX_TABLE; }
	virtual void setINDEX(int i) { INDEX_TABLE = i; }
	virtual void setOffset(int off) { offset = off; }
	virtual int getOffset() { return offset; }
	virtual void setAssigned() { hasValueorAddr = true; }

protected:
	int category = 0;
	// 0 -> var, 1 -> const, 2 -> func
	int type = 0;
	// -1 -> void, 0 -> int, 1 -> char, also records the return type of a function
	std::string name;
	int INDEX_TABLE = -1;
	int offset = -1;
	bool hasValueorAddr = false;
};


class ArraySymEntry : public SymTableEntry {
public:
	ArraySymEntry(std::string name, int cate, int type, int size):
		SymTableEntry(name, cate, type), 
		dim(1), firstDimSize(size), secDimSize(-1) {}
	ArraySymEntry(std::string name, int cate, int type, int size1, int size2) :
		SymTableEntry(name, cate, type),
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
	ScalerSymEntry(std::string name, int cate, int type):
		SymTableEntry(name, cate, type) {}
	
	int value = -1;
};

class FormalVarSymEntry : public ScalerSymEntry {
public:
	FormalVarSymEntry(std::string name, int cate, int type, int parFuncPos_):
		ScalerSymEntry(name, cate, type), parFuncPos(parFuncPos_) {}

	int getPAR_FUNC_POS() { return parFuncPos; }
	void setPAR_FUNC_POS(int pos) { parFuncPos = pos; }

private:
	int parFuncPos = -1;
};

class FuncSymEntry : public SymTableEntry {
public:
	FuncSymEntry(std::string name, int cate, int type, int argnum=0) :
		SymTableEntry(name, cate, type), argNum(argnum) {}

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
	std::string funcName;
	std::vector<SymbolTable*> children;
	SymbolTable* father;

	SymbolTable(std::string name) : funcName(name) {}

	void insertSymbolEntry(SymTableEntry* sym);

	bool duplicateName(std::string name);
	SymTableEntry* getSymByName(std::string name);

	void printSymTable(std::ostream& os);

	SymbolTable* findTableByName(std::string name) {
		if (funcName == name) {
			return this;
		}
		else {
			for (auto i : children) {
				SymbolTable* tem = i->findTableByName(name);
				if (tem != nullptr) {
					return tem;
				}
			}
			return nullptr;
		}
	}

	
private:
	std::vector<SymTableEntry*> symTable;
	
};

class EnvTable {
public:
	EnvTable() {
		root = new SymbolTable("global");
		top = root;
	}

	SymbolTable *root;
	SymbolTable *top;

	void popTable() {
		top = top->father;
	}

	void addTable(std::string name) {
		SymbolTable* newTab = new SymbolTable(name);
		newTab->father = top;
		top->children.push_back(newTab);
		top = newTab;
	}

	SymbolTable* getTableByFuncName(std::string name) {
		return root->findTableByName(name);
	}

	SymTableEntry* getSymByName(std::string name) {
		SymbolTable* p = top;
		SymTableEntry* tem = nullptr;
		while (p != root) {
			tem = p->getSymByName(name);
			if (tem != nullptr) {
				return tem;
			}
			p = p->father;
		}
		tem = root->getSymByName(name);
		if (tem != nullptr) {
			return tem;
		}
		return nullptr;
	}

	bool checkDuplicate(std::string name) {
		return top->duplicateName(name);
	}

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