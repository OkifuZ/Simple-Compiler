#ifndef SYNTREE_H_INCLUDED
#define SYNTREE_H_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include "lexical.h"
#include "parser.h"
#include "errHand.h"


enum class TYPE_NTS {
	ERROR = -2, OTHERS = -1,

	STRING = 0, UNSIGNED_INT, INT,
	CONST_DEF, CONST_DEC, DEC_HEAD,
	CONSTANT, VARDEF_WITH_INIT, VAR_DEFWIOU_INIT, VAR_DEC,
	VAR_DEF, ARGLIST, TERM,
	FACTOR, EXPERSSION, SENTENCE,
	CALL_REFUNC_SEN, CALL_NONREFUNC_SEN,
	REFUNC_DEF, NONREFUNC_DEF, VALUE_ARGLIST, ASSIGN_SEN,
	IFELSE_SEN, CONDITION, STEP_LEN, LOOP_SEN,
	CASE_SEN, CASE_LIST, SWITCH_SEN, DEFAULT,
	WRITE_SEN, READ_SEN, RETURN_SEN, SEN_LIST,
	COMPOUND_SEN,
	MAIN, PROGRAM,

	IDEN, CHAR, TYPEIDEN, COMPARE_OP, ARRAY_CONST,
	DBARRAY_CONST, ONED_DEC
};

extern const std::vector<std::string> STRING_NTS_LIST;
extern const std::string get_STRING_NTS(TYPE_NTS type);

class SynNode {
public:

	SynNode(std::string name_) : 
		name(name_) {}

	virtual void print(std::ostream&) = 0;
	virtual std::string getName() const { return name; }
	virtual ~SynNode() = default;

protected:
	std::string name;
};


class TerNode : public SynNode {
public:
	TerNode(LexSymbol sym_, int iv = 0, char cv = '\0') :
		SynNode(sym_.token, iv, cv), symbol(sym_) {}
	
	void print(std::ostream&) override;

	int getLine() const { return symbol.line; }
	TYPE_SYM getType() const { return symbol.type; }

private:
	LexSymbol symbol;
};


class NonTerNode : public SynNode {
public:
	NonTerNode(TYPE_NTS type_, bool high_, int iv = 0, char cv = '\0') :
		SynNode(get_STRING_NTS(type_), iv, cv), type(type_), highlighted(high_) {}
	
	void print(std::ostream&) override;
	void addChild(SynNode* s) { children.push_back(s); }
	TYPE_NTS getType() const { return type; }
	SynNode* getLastChild() {
		return children.back();
	}

private:
	TYPE_NTS type;
	bool highlighted;
	std::vector<SynNode*> children;
};

#endif // SYNTREE_H_INCLUDED