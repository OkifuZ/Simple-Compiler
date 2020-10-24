#include "../header/lexical.h"
#include "../header/parser.h"
#include "../header/synTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

/*CLASS: PARSER*/

using namespace std;

// pos -1 is symbol's index in record
// simple case
// 1 2 3 4 5 6
//     p       t
// empty case
//
// p(t)
// traceback case
// 1 2 3 4 5 6
//   p t
// cache: pos, pos+1, pos+2, ... , top-1; empty when top==pos

bool Parser::nextSym()
{
    if (pos < top)
    {
        symbol = record[pos];
        pos++;
    }
    else
    {
        preReadSym(1);
        symbol = record[pos];
        pos++;
    }
    this->flushed = (top == pos);
    if (symbol.type == TYPE_SYM::ERROR)
    {
        addErrorMessage(symbol.line, 'a');
    }
    return !(symbol.type == TYPE_SYM::ENDS || symbol.type == TYPE_SYM::ERROR);
}

void Parser::preReadSym(int time)
{
    this->flushed = false;
    while (time--)
    {
        if (top < record.size())
            top++;
        else
        {
            string token;
            TYPE_SYM type = this->lexicalAnalyzer.getsym(token);
            LexSymbol sym = LexSymbol(type, token);
            record.push_back(sym);
            top++;
        }
    }
}

void Parser::flushPreRead()
{
    top = pos;
    this->flushed = true;
}

bool Parser::cacheContainsSym(TYPE_SYM type)
{
    for (int i = pos; i < top; i++)
    {
        if (record[i].type == type)
            return true;
    }
    return false;
}

void Parser::printError(ostream &out)
{
#ifdef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList)
    {
        out << item.line << ' ' << item.iden << ' ' << item.message << '\n';
    }
#endif //
#ifndef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList)
    {
        out << item.line << ' ' << item.iden << '\n';
    }
#endif // !PRINT_ERROR_MESSAGE
}

inline SynNode *Parser::stringP(string *attr_str_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::STRING, true);
    if (symbol.type == TYPE_SYM::STRCON)
    {
        node->addChild(new TerNode(symbol));
        *attr_str_syn = symbol.token;
    }
    else
    {
        printPos(881779);
    }
    nextSym();
    return node;
}

inline SynNode *Parser::unsignedIntP(int *attr_int_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::UNSIGNED_INT, true);
    if (symbol.type == TYPE_SYM::INTCON)
    {
        *attr_int_syn = str2int(symbol.token);
        node->addChild(new TerNode(symbol));
    }
    else
    {
        printPos(231313);
    }
    nextSym();
    return node;
}

inline SynNode *Parser::intP(int *attr_int1_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::INT, true);
    bool attr_isNeg = false;
    if (this->symbol.type == TYPE_SYM::PLUS ||
        this->symbol.type == TYPE_SYM::MINU)
    {
        attr_isNeg = (this->symbol.type == TYPE_SYM::MINU);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    int attr_int2_syn;
    node->addChild(unsignedIntP(&attr_int2_syn));
    *attr_int1_syn = attr_isNeg ? -attr_int2_syn : attr_int2_syn;
    return node;
}

inline SynNode *Parser::charP(char *attr_char_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::CHAR, false);
    if (symbol.type == TYPE_SYM::CHARCON)
    {
        node->addChild(new TerNode(symbol));
        *attr_char_syn = str2char(symbol.token);
    }
    else
    {
        printPos(313);
    }
    nextSym();
    return node;
}

inline SynNode *Parser::constDefP(int layer)
{
    int attr_cate_inh = _CAT_CONST, attr_type_syn = -1, LAYER = layer, attr_intLine_syn = 0;
    string attr_strName_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEF, true);
    if (symbol.type == TYPE_SYM::INTTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_INT;
        nextSym();
        node->addChild(idenP(&attr_strName_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strName_syn, LAYER))
        {
            addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
        }
        if (!(symbol.type == TYPE_SYM::ASSIGN))
        {
            printPos(414123);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        int attr_int_syn;
        node->addChild(intP(&attr_int_syn));
        symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&attr_strName_syn, &attr_intLine_syn));
            if (checkDuplicate(attr_strName_syn, LAYER))
            {
                addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(97135);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(intP(&attr_int_syn));
            symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        }
    }
    else if (this->symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_CHAR;
        nextSym();
        node->addChild(idenP(&attr_strName_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strName_syn, LAYER))
        {
            addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
        }
        if (!(this->symbol.type == TYPE_SYM::ASSIGN))
        {
            printPos(3);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        char attr_char_syn;
        node->addChild(charP(&attr_char_syn));
        symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        while (this->symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&attr_strName_syn, &attr_intLine_syn));
            if (checkDuplicate(attr_strName_syn, LAYER))
            {
                addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(97135);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(charP(&attr_char_syn));
            symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        }
    }
    else
    {
        printPos(4123123);
    }
    return node;
}

inline SynNode *Parser::constDecP(int layer)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEC, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(LAYER));
        if (!(symbol.type == TYPE_SYM::SEMICN))
        {
            addErrorMessage(symbol.line, 'k', "常量定义中缺少分号");
            printPos(76262);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(6381);
    }
    while (this->symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(LAYER));
        if (!(this->symbol.type == TYPE_SYM::SEMICN))
        {
            addErrorMessage(symbol.line, 'k', "常量定义中缺少分号");
            printPos(51242);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    return node;
}

inline SynNode *Parser::idenP(string *attr_strName_syn, int *attr_intLine_syn)
{ // both default = nullptr
    NonTerNode *node = new NonTerNode(TYPE_NTS::IDEN, false);
    if (!(symbol.type == TYPE_SYM::IDENFR))
    {
        printPos(22);
    }
    *attr_strName_syn = symbol.token;
    *attr_intLine_syn = symbol.line;
    node->addChild(new TerNode(symbol));
    nextSym();
    return node;
}

inline SynNode *Parser::decHeadP(string *attr_strNmae_syn, int *attr_intType_syn, int *attr_intLine_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::DEC_HEAD, true);
    if (this->symbol.type == TYPE_SYM::INTTK ||
        this->symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        *attr_intLine_syn = symbol.line;
        node->addChild(idenP(attr_strNmae_syn));
        *attr_intType_syn = symbol.type == TYPE_SYM::INTTK ? _TYPE_INT : _TYPE_CHAR;
    }
    else
    {
        printPos(3313);
    }
    return node;
}

inline SynNode *Parser::constP(int *attr_int_syn, int* attr_intTYpe_syn, int *attr_intLine_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONSTANT, true);
    if (symbol.type == TYPE_SYM::CHARCON)
    {
        node->addChild(new TerNode(symbol));
        *attr_intLine_syn = symbol.line;
        char* attr_temChar;
        node->addChild(charP(attr_temChar));
        *attr_int_syn = *attr_temChar;
        *attr_intTYpe_syn = _TYPE_CHAR;
        nextSym();
    }
    else
    {
        *attr_intLine_syn = symbol.line;
        node->addChild(intP(attr_int_syn));
        *attr_intTYpe_syn = _TYPE_INT;
    }
    return node;
}

inline SynNode *Parser::arrayConstP(int attr_size_inh, int *attr_intType_syn, int*attr_intLine_syn)
{
    int i = 0, attr_temType;
    int attr_conVal_syn;
    int attr_intLine_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP(&attr_conVal_syn, attr_intType_syn, &attr_intLine_syn));
        i++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            i++;
            if (i > attr_size_inh)
            {
                printPos(99965);
                addErrorMessage(symbol.line, 'n', "一维数组初始化个数不匹配");
            }
            nextSym();
            node->addChild(constP(&attr_conVal_syn, &attr_temType, &attr_intLine_syn));
            if (attr_temType != *attr_intType_syn)
            {
                *attr_intType_syn = _TYPE_ERROR;
            }
        }
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(99046);
        }
        *attr_intLine_syn = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(25261);
    }
    
    return node;
}

inline SynNode *Parser::doubleArrayConstP
(int attr_size1_inh, int attr_size2_inh, int *attr_intType_syn, int* attr_intLine_syn)
{
    int i = 0, attr_temType;
    NonTerNode *node = new NonTerNode(TYPE_NTS::DBARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arrayConstP(attr_size2_inh, attr_intType_syn));
        i++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(attr_size2_inh, &attr_temType));
            if (attr_temType != *attr_intType_syn)
            {
                *attr_intType_syn = _TYPE_ERROR;
            }
            i++;
            if (i > attr_size1_inh)
            {
                printPos(91651);
                addErrorMessage(symbol.line, 'n', "二维数组初始化维数不匹配");
            }
        }
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(84762);
        }
        *attr_intLine_syn = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(151515);
    }
    return node;
}

inline SynNode *Parser::oneDdeclareP(int *attr_size_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::ONED_DEC, false);
    if (symbol.type == TYPE_SYM::LBRACK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::CHARCON) {
            addErrorMessage(symbol.line, 'i', "数组定义时下标为字符型");
        }
        else {
            node->addChild(unsignedIntP(attr_size_syn));
        }
        if (!(symbol.type == TYPE_SYM::RBRACK))
        {
            printPos(36536);
            addErrorMessage(symbol.line, 'm', "一维数组定义无右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(42424);
    }
    return node;
}

inline SynNode *Parser::varDerWithInitP(int layer, int attr_intType_inh)
{
    int attr_cate_inh = _CAT_VAR, LAYER = layer;
    int attr_size1_syn, attr_size2_syn, attr_intLine_syn = 0;
    int attr_value_syn, attr_conType_syn;
    string attr_strNmae_syn;
    bool hasASSIGN = false;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VARDEF_WITH_INIT, true);
    node->addChild(idenP(&attr_strNmae_syn, &attr_intLine_syn));
    if (checkDuplicate(attr_strNmae_syn, LAYER))
    {
        addErrorMessage(attr_intLine_syn, 'b', "有初始化的变量定义时名字重定义");
    }
    if (symbol.type == TYPE_SYM::ASSIGN)
    {
        hasASSIGN = true;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP(&attr_value_syn, &attr_conType_syn, &attr_intLine_syn));
        if (attr_intType_inh != attr_conType_syn)
        {
            addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化标量常量类型不一致");
        }
        symbolTable.insertSymbolEntry(
            new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER));
    }
    else if (symbol.type == TYPE_SYM::LBRACK)
    {
        node->addChild(oneDdeclareP(&attr_size1_syn));
        if (symbol.type == TYPE_SYM::ASSIGN)
        {
            hasASSIGN = true;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(attr_size1_syn, &attr_conType_syn, &attr_intLine_syn));
            if (attr_intType_inh != attr_conType_syn)
            {
                addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化一维数组常量类型不一致");
            }
            symbolTable.insertSymbolEntry(
                new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn));
        }
        else if (symbol.type == TYPE_SYM::LBRACK)
        {
            node->addChild(oneDdeclareP(&attr_size2_syn));
            if (symbol.type == TYPE_SYM::ASSIGN)
            {
                hasASSIGN = true;
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(doubleArrayConstP(attr_size1_syn, attr_size2_syn, &attr_conType_syn, &attr_intLine_syn));
                if (attr_intType_inh != attr_conType_syn)
                {
                    addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化二维数组常量类型不一致");
                }
                symbolTable.insertSymbolEntry(
                    new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn, attr_size2_syn));
            }
            else
            {
                printPos(55255); // 二维数组无初始化
            }
        }
        else
        {
            printPos(62673); // 一维数组无初始化
        }
    }
    else {
        printPos(16161); //标量无初始化
    }
    if (!hasASSIGN)
    {
        addErrorMessage(symbol.line, 'o', "变量定义及初始化没有赋值");
    }
    return node;
}

inline SynNode *Parser::varDerWithoutInitP(int layer, int attr_intType_inh)
{
    int attr_cate_inh = _CAT_VAR, LAYER = layer, attr_size1_syn, attr_size2_syn;
    int attr_intLine_syn = 0;
    string attr_strNmae_syn;
    string attr_strNmae_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VAR_DEFWIOU_INIT, true);
    node->addChild(idenP(&attr_strNmae_syn, &attr_intLine_syn));
    if (checkDuplicate(attr_strNmae_syn, LAYER))
    {
        addErrorMessage(attr_intLine_syn, 'b', "无初始化变量定义时名字重定义");
    }
    int dim = 0;
    if (symbol.type == TYPE_SYM::LBRACK)
    {
        dim++;
        node->addChild(oneDdeclareP(&attr_size1_syn));
        if (symbol.type == TYPE_SYM::LBRACK)
        {
            dim++;
            node->addChild(oneDdeclareP(&attr_size2_syn));
        }
    }
    if (dim == 0)
    {
        symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER));
    }
    else if (dim == 1)
    {
        symbolTable.insertSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER, attr_size1_syn));
    }
    else if (dim == 2)
    {
        symbolTable.insertSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER, attr_size1_syn, attr_size2_syn));
    }
    while (symbol.type == TYPE_SYM::COMMA)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(&attr_strNmae_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strNmae_syn, LAYER))
        {
            addErrorMessage(attr_intLine_syn, 'b', "无初始化变量定义时名字重定义");
        }
        dim = 0;
        if (symbol.type == TYPE_SYM::LBRACK)
        {
            dim++;
            node->addChild(oneDdeclareP(&attr_size1_syn));
            if (symbol.type == TYPE_SYM::LBRACK)
            {
                dim++;
                node->addChild(oneDdeclareP(&attr_size2_syn));
            }
        }
        if (dim == 0)
        {
            symbolTable.insertSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER));
        }
        else if (dim == 1)
        {
            symbolTable.insertSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER, attr_size1_syn));
        }
        else if (dim == 2)
        {
            symbolTable.insertSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intLine_syn, LAYER, attr_size1_syn, attr_size2_syn));
        }
    }
    return node;
}

SynNode *Parser::varDecP(int layer)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VAR_DEC, true);
    node->addChild(varDefP(LAYER));
    if (!(symbol.type == TYPE_SYM::SEMICN))
    {
        printPos(9146);
        addErrorMessage(symbol.line, 'k', "变量说明缺少分号");
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    while (true)
    {
        preReadSym(2);
        if (!cacheContainsSym(TYPE_SYM::LPARENT) &&
            (symbol.type == TYPE_SYM::CHARTK ||
             symbol.type == TYPE_SYM::INTTK))
        {
            flushPreRead();
            node->addChild(varDefP(LAYER));
            if (!(symbol.type == TYPE_SYM::SEMICN))
            {
                printPos(22431);
                addErrorMessage(symbol.line, 'k', "变量说明缺少分号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
        {
            flushPreRead();
            break;
        }
    }
    return node;
}

inline SynNode *Parser::varDefP(int layer)
{
    int attr_intType_tem, LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VAR_DEF, true);
    node->addChild(typeIdenP(&attr_intType_tem));
    int i = 0;
    while (true)
    {
        i++;
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::SEMICN) ||
            cacheContainsSym(TYPE_SYM::COMMA))
        {
            flushPreRead();
            node->addChild(varDerWithoutInitP(LAYER, attr_intType_tem));
            break;
        }
        if (cacheContainsSym(TYPE_SYM::ASSIGN))
        {
            flushPreRead();
            node->addChild(varDerWithInitP(LAYER, attr_intType_tem));
            break;
        }
        if (i >= 9)
        {
            flushPreRead();
            printPos(998613);
            break;
        }
    }
    return node;
}

inline SynNode *Parser::typeIdenP(int* attr_intType_syn)
{ // no highlight
    NonTerNode *node = new NonTerNode(TYPE_NTS::TYPEIDEN, false);
    if (symbol.type == TYPE_SYM::INTTK)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::CHARTK)
    {
        *attr_intType_syn = _TYPE_CHAR;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    {
        printPos(51749);
    }
    return node;
}

inline SynNode *Parser::termP(int layer, int* attr_intType_syn, int* attr_value_syn)
{
    int LAYER = layer, attr_type_tem, attr_value_tem, attr_value_ans = 1;
    *attr_intType_syn = _TYPE_CHAR; // set type default as char
    NonTerNode *node = new NonTerNode(TYPE_NTS::TERM, true);
    node->addChild(factorP(LAYER, &attr_type_tem, &attr_value_tem));
    attr_value_ans *= attr_value_tem;
    if (attr_type_tem == _TYPE_INT)
    {
        *attr_intType_syn = _TYPE_INT;
    }
    while (symbol.type == TYPE_SYM::MULT || symbol.type == TYPE_SYM::DIV)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(factorP(LAYER, &attr_type_tem, &attr_value_tem));
        attr_value_ans = (symbol.type == TYPE_SYM::MULT ? 
            attr_value_ans * attr_value_tem : attr_value_ans / attr_type_tem);
    }
    *attr_value_syn = attr_value_ans;
    return node;
}

inline SynNode *Parser::factorP(int layer, int* attr_intType_syn, int* attr_value_syn)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::FACTOR, true);
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP(LAYER, attr_intType_syn, attr_value_syn));
        if (!(symbol.type == TYPE_SYM::RPARENT))
        {
            printPos(914151);
            addErrorMessage(symbol.line, 'l', "因子中缺少右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::CHARCON)
    {
        char attr_char_syn;
        node->addChild(charP(&attr_char_syn));
        *attr_value_syn = static_cast<int>(attr_char_syn);
        *attr_intType_syn = _TYPE_CHAR;
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::PLUS ||
             symbol.type == TYPE_SYM::MINU || symbol.type == TYPE_SYM::INTCON)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(intP(attr_value_syn));
    }
    else if (symbol.type == TYPE_SYM::IDENFR)
    {
        preReadSym(1);
        if (!cacheContainsSym(TYPE_SYM::LPARENT))
        {
            flushPreRead();
            node->addChild(referenceP(LAYER, attr_intType_syn, attr_value_syn, false, 0));
        }
        else if (cacheContainsSym(TYPE_SYM::LPARENT))
        {
            flushPreRead();
            node->addChild(callFuncSenP(LAYER, attr_intType_syn, attr_value_syn));
        }
    }
    else
        printPos(99517);
    return node;
}

inline SynNode *Parser::expressionP(int layer, int* attr_intType_syn, int* attr_value_syn)
{
    int LAYER = layer, attr_type_tem, attr_value_tem, attr_value_ans = 0;
    *attr_intType_syn = _TYPE_CHAR;
    NonTerNode *node = new NonTerNode(TYPE_NTS::EXPERSSION, true);
    if (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    node->addChild(termP(LAYER, &attr_type_tem, &attr_value_tem));
    attr_value_ans += attr_value_tem;
    if (attr_type_tem == _TYPE_INT) {
        *attr_intType_syn = _TYPE_INT;
    }
    while (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(termP(LAYER, &attr_type_tem, &attr_value_tem));
        attr_value_ans = (symbol.type == TYPE_SYM::PLUS ? 
            attr_value_ans + attr_value_tem : attr_value_ans - attr_type_tem);
    }
    *attr_value_syn = attr_value_ans;
    return node;
}

SynNode* Parser::referenceP(int layer, int* attr_intType_syn, int* attr_value_syn, bool isAssign, int attr_assignVal_inh=0) {
    string attr_strName_syn;
    int attr_intLine_syn, LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::REFERENCE, false);
    node->addChild(idenP(&attr_strName_syn, &attr_intLine_syn));
    SymTableEntry* attr_sym = getEntryByName(attr_strName_syn);
    if (attr_sym == nullptr) {
        addErrorMessage(symbol.line, 'c', "引用了未定义的名字");
    }
    else if (isAssign && attr_sym->getCATE() == _CAT_CONST) {
        addErrorMessage(symbol.line, 'j', "改变常量的值");
    } 
    else {
        *attr_intType_syn = attr_sym->getTYPE();
        if (symbol.type == TYPE_SYM::LBRACK)
        {
            node->addChild(new TerNode(symbol));
            int attr_subValue_syn, attr_subType_syn;
            nextSym();
            node->addChild(expressionP(LAYER, &attr_subType_syn, &attr_subValue_syn)); 
            if (attr_subType_syn == _TYPE_CHAR)
            {
                addErrorMessage(symbol.line, 'i', "数组下标为字符型");
            }
            if (!(symbol.type == TYPE_SYM::RBRACK))
            {
                printPos(6516151);
                addErrorMessage(symbol.line, 'm', "数组元素缺少右中括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            if (symbol.type == TYPE_SYM::LBRACK)
            {
                node->addChild(new TerNode(symbol));
                int attr_subValue_syn, attr_subType_syn;
                nextSym();
                node->addChild(expressionP(LAYER, &attr_subType_syn, &attr_subValue_syn)); 
                if (attr_subType_syn == _TYPE_CHAR)
                {
                    addErrorMessage(symbol.line, 'i', "数组下标为字符型");
                }
                if (!(symbol.type == TYPE_SYM::RBRACK))
                {
                    printPos(881651);
                    addErrorMessage(symbol.line, 'm', "二维数组元素缺少右中括号");
                }
                node->addChild(new TerNode(symbol));
                nextSym();
            }
        }
        else
        {
            // just var scaler
        }
    }
    if (attr_value_syn != nullptr) {
        *attr_value_syn = 0; // so for no access to memory
    }
}

inline SynNode *Parser::sentenceP(int layer, int type)
{ // TODO
    int LAYER = layer, LINE = -1;
    bool hasReturn = false;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SENTENCE, true);
    if (symbol.type == TYPE_SYM::WHILETK || symbol.type == TYPE_SYM::FORTK)
    {
        node->addChild(loopSenP(LAYER + 1));
    }
    else if (symbol.type == TYPE_SYM::IFTK)
    {
        node->addChild(ifelseSenP(LAYER + 1));
    }
    else if (symbol.type == TYPE_SYM::SCANFTK)
    {
        node->addChild(readSenP());
        if (!(symbol.type == TYPE_SYM::SEMICN))
        {
            LINE = symbol.line;
            addErrorMessage(LINE, 'k', "缺少分号");
            printPos(883833);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::PRINTFTK)
    {
        node->addChild(writeSenP(LAYER));
        if (!(symbol.type == TYPE_SYM::SEMICN))
        {
            LINE = symbol.line;
            addErrorMessage(LINE, 'k', "缺少分号");
            printPos(426621);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        node->addChild(switchSenP(LAYER + 1));
    }
    else if (symbol.type == TYPE_SYM::RETURNTK)
    {
        hasReturn = true;
        LINE = symbol.line;
        node->addChild(returnSenP(LAYER, type));
        if (!(symbol.type == TYPE_SYM::SEMICN))
        {
            LINE = symbol.line;
            addErrorMessage(LINE, 'k', "缺少分号");
            printPos(99578);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::SEMICN)
    {
        LINE = symbol.line;
        addErrorMessage(LINE, 'k', "缺少分号");
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::IDENFR)
    {
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::LPARENT))
        {
            flushPreRead();
            node->addChild(callFuncSenP(LAYER));
        }
        else
        {
            flushPreRead();
            node->addChild(assignSenP(LAYER));
        }
        if (!(symbol.type == TYPE_SYM::SEMICN))
        {
            LINE = symbol.line;
            addErrorMessage(LINE, 'k', "缺少分号");
            printPos(88174);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceListP(LAYER + 1));
        if (!(symbol.type == TYPE_SYM::RBRACE))
            printPos(737892);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
        printPos(6262111);
    if (type != TYPE_VOID && !hasReturn)
    { // TODO
        addErrorMessage(LINE, 'h', "有返回值函数不存在return语句");
    }
    return node;
}

inline SynNode *Parser::arguListP(int layer, int *attr_num_syn, FuncSymEntry *func)
{
    int LAYER = layer;
    int attr_cate = _CAT_VAR, attr_line = 0, n = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ARGLIST, true);
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK)
    {
        int attr_type_tem;
        node->addChild(typeIdenP(&attr_type_tem));
        node->addChild(idenP(&attr_strName, &attr_line));
        // 这里应该不用检查重复
        symbolTable.insertSymbolEntry(
            new FormalVarSymEntry(attr_strName, attr_cate, attr_type_tem, LAYER, func->getINDEX()));
        func->addParaType(attr_type_tem);
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(typeIdenP(&attr_type_tem));
            node->addChild(idenP(&attr_strName, &attr_line));
            if (checkDuplicate(attr_strName, LAYER)) 
            {
                addErrorMessage(attr_line, 'b', "参数列表中重复定义名字");
            }
            symbolTable.insertSymbolEntry(
                new FormalVarSymEntry(attr_strName, attr_cate, attr_type_tem, LAYER, func->getINDEX()));
            func->addParaType(attr_type_tem);
            n++;
        }
    }
    *attr_num_syn = n;
    return node;
}

SynNode *Parser::refuncDefineP(int layer)
{
    int LAYER = layer, attr_type, attr_cate = _CAT_FUNC, attr_line = -1, attr_argnum = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::REFUNC_DEF, true);
    node->addChild(decHeadP(&attr_strName, &attr_type, &attr_line));
    if (checkDuplicate(attr_strName, LAYER))
    {
        addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
    }
    FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_cate, attr_type, LAYER, 0);
    symbolTable.insertSymbolEntry(symFUNC);
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arguListP(LAYER, &attr_argnum, symFUNC));
        if (!(symbol.type == TYPE_SYM::RPARENT))
        {
            printPos(123456);
            addErrorMessage(symbol.line, 'l', "nonvoid函数定义缺少右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(22009);
    }
    symFUNC->setARGNUM(attr_argnum);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(compoundSenP(LAYER + 1, true, attr_type));
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(891947);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    {
        printPos(91415);
    }
    return node;
}

SynNode *Parser::nonrefuncDefineP(int layer)
{
    int LAYER = layer, attr_type = _TYPE_VOID, attr_cate = _CAT_FUNC, attr_line = -1, attr_argnum = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::NONREFUNC_DEF, true);
    if (symbol.type == TYPE_SYM::VOIDTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(&attr_strName, &attr_line));
        if (checkDuplicate(attr_strName, LAYER))
        {
            addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
        }
        FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_line, attr_line, LAYER, 0);
        symbolTable.insertSymbolEntry(symFUNC);
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arguListP(LAYER, &attr_argnum, symFUNC));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(8085);
                addErrorMessage(symbol.line, 'l', "void函数定义缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
        {
            printPos(8633);
        }
        symFUNC->setARGNUM(attr_argnum);
        if (symbol.type == TYPE_SYM::LBRACE)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(compoundSenP(LAYER + 1, true, attr_type));
            if (!(symbol.type == TYPE_SYM::RBRACE))
            {
                printPos(9976);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else{
            printPos(9842);
        }
    }
    else
    {
        printPos(7522);
    }
    return node;
}

inline SynNode *Parser::callFuncSenP(int layer, int* attr_intType_syn, int* attr_value_syn)
{
    int LAYER = layer, attr_line = -1;
    string attr_strName;
    SynNode *iden = idenP(&attr_strName, &attr_line);
    NonTerNode *node = nullptr;
    FuncSymEntry *func = getFUNC_CALL(attr_strName);
    if (func == nullptr)
    {
        addErrorMessage(attr_line, 'c', "函数调用了未定义的名字");
    }
    else
    {
        if (func->getTYPE() == _TYPE_VOID)
        {
            node = new NonTerNode(TYPE_NTS::CALL_NONREFUNC_SEN, true);
        }
        else
        {
            node = new NonTerNode(TYPE_NTS::CALL_REFUNC_SEN, true);
        }
        node->addChild(iden);
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(valueArgueListP(LAYER, func));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(99875);
                addErrorMessage(symbol.line, 'l', "函数调用缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            printPos(88278);
        }
    }
    *attr_value_syn = 0; // so far no compute enabled
    return node;
}

inline SynNode *Parser::valueArgueListP(int layer, FuncSymEntry *func)
{
    int LAYER = layer, attr_argnum = func->getARGNUM(), n = 0, attr_type_tem, attr_intLine = -1, attr_value_tem;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VALUE_ARGLIST, true);
    if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU ||
        symbol.type == TYPE_SYM::IDENFR ||
        symbol.type == TYPE_SYM::LPARENT ||
        symbol.type == TYPE_SYM::LBRACK ||
        symbol.type == TYPE_SYM::INTCON ||
        symbol.type == TYPE_SYM::CHARCON)
    {
        node->addChild(expressionP(LAYER, &attr_type_tem, &attr_value_tem));
        if (func->getParaTypeList().empty() || n >= func->getARGNUM())
        {
            addErrorMessage(symbol.line, 'd', "函数调用参数个数不匹配");
        }
        else if (attr_type_tem != func->getParaTypeList()[n])
        {
            addErrorMessage(symbol.line, 'e', "函数调用参数类型不匹配");
        }
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER, &attr_type_tem, &attr_value_tem));
            if (func->getParaTypeList().empty() || n >= func->getARGNUM())
            {
                addErrorMessage(symbol.line, 'd', "函数调用参数个数不匹配");
            }
            else if (attr_type_tem != func->getParaTypeList()[n])
            {
                addErrorMessage(symbol.line, 'e', "函数调用参数类型不匹配");
            }
            n++;
        }
    }
    if (func->getARGNUM() != n)
    {
        addErrorMessage(symbol.line, 'd', "函数调用个数不匹配");
    }
    return node;
}

inline SynNode *Parser::assignSenP(int layer)
{
    int LAYER = layer, attr_leftType, attr_rightType, attr_rightValue;
    SymTableEntry *attr_leftSym;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ASSIGN_SEN, true);
    node->addChild(referenceP(LAYER, &attr_leftType, nullptr, true, 0));
    if (symbol.type == TYPE_SYM::ASSIGN)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP(LAYER, &attr_rightType, &attr_rightValue));
    }
    else
    {
        printPos(62562);
    }
    return node;
}

inline SynNode *Parser::ifelseSenP(int layer, bool inFunc, int attr_retType, int* attr_retNum_syn)
{
    int LAYER = layer, attr_intLine = -1;
    bool attr_res;
    NonTerNode *node = new NonTerNode(TYPE_NTS::IFELSE_SEN, true);
    if (symbol.type == TYPE_SYM::IFTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER, &attr_res));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(5662325);
                addErrorMessage(symbol.line, 'l', "ifelse中缺失右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
            printPos(66725);
    }
    else
        printPos(9947252);
    node->addChild(sentenceP(LAYER, inFunc, attr_retType));
    if (symbol.type == TYPE_SYM::ELSETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER, inFunc, attr_retType));
    }
    return node;
}

inline SynNode *Parser::compareOpP(int* attr_op)   
{// 0 <; 1 <=; 2 >; 3 >=; 4 ==; 5 !=
    NonTerNode *node = new NonTerNode(TYPE_NTS::COMPARE_OP, false);
    switch(symbol.type) {
        case TYPE_SYM::LSS:
            *attr_op = 0;
            break;
        case TYPE_SYM::LEQ:
            *attr_op = 1;
            break;
        case TYPE_SYM::GRE:
            *attr_op = 2;
            break;
        case TYPE_SYM::GEQ:
            *attr_op = 3;
            break;
        case TYPE_SYM::EQL:
            *attr_op = 4;
            break;
        case TYPE_SYM::NEQ:
            *attr_op = 5;
            break;
        default:
            printPos(4946561);
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    return node;
}

inline SynNode *Parser::conditionP(int layer, bool* attr_result)
{
    int LAYER = layer, attr_leftType, attr_rightType, attr_leftVal, attr_rightVal, attr_op;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONDITION, true);
    node->addChild(expressionP(LAYER, &attr_leftType, &attr_leftVal));
    if (attr_leftType == _TYPE_CHAR)
    {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    node->addChild(compareOpP(&attr_op));
    node->addChild(expressionP(LAYER, &attr_rightType, &attr_rightVal));
    if (attr_rightType == _TYPE_CHAR)
    {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    *attr_result = false;
    return node;
}

inline SynNode *Parser::stepLengthP(int* attr_len)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::STEP_LEN, true);
    node->addChild(unsignedIntP(attr_len));
    return node;
}

inline SynNode *Parser::loopSenP(int layer, bool isFunc, int attr_type_inh, int* attr_retNum_syn)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::LOOP_SEN, true);
    if (symbol.type == TYPE_SYM::WHILETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            bool attr_res;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER, &attr_res));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(11451);
                addErrorMessage(symbol.line, 'l', "while语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else{
            printPos(414151);
        }
        node->addChild(sentenceP(LAYER, isFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::FORTK)
    {
        string attr_strName;
        SymTableEntry *sym = nullptr;
        int attr_line, attr_intType, attr_val;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            // phase 1
            node->addChild(referenceP(LAYER, &attr_intType, &attr_val, true, 0));
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(841656);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            int attr_expVal, attr_expType;
            node->addChild(expressionP(LAYER, &attr_expType, &attr_expVal));
            if (!(symbol.type == TYPE_SYM::SEMICN))
            {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号1");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            // phase 2
            bool attr_res;
            node->addChild(conditionP(LAYER, &attr_res));
            if (!(symbol.type == TYPE_SYM::SEMICN))
            {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号2");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(referenceP(LAYER, &attr_intType, &attr_val, true, 0));
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(61319);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            // phase 3
            node->addChild(referenceP(LAYER, &attr_intType, &attr_val, false, 0));
            if (!(symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU))
            {
                printPos(26262);
            }
            node->addChild(new TerNode(symbol));
            nextSym();

            int attr_len;
            node->addChild(stepLengthP(&attr_len));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(911451);
                addErrorMessage(symbol.line, 'l', "for语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
        {
            printPos(907691);
        }
        node->addChild(sentenceP(LAYER, isFunc, attr_type_inh, attr_retNum_syn));
    }
    return node;
}

inline SynNode *Parser::switchSenP(int layer, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer, attr_expType, attr_val;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SWITCH_SEN, true);
    if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER, &attr_expType, &attr_val));
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(77251);
                addErrorMessage(symbol.line, 'l', "switch缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
        {
            printPos(14149);
        }
        if (symbol.type == TYPE_SYM::LBRACE)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(caseListP(LAYER, attr_expType, isFunc, attr_retType_inh, attr_retNum_syn));
            node->addChild(defaultP(LAYER, isFunc, attr_retType_inh, attr_retNum_syn));
            if (!(symbol.type == TYPE_SYM::RBRACE))
            {
                printPos(214156);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
            printPos(991737);
    }
    else
        printPos(626666);
    return node;
}

inline SynNode *Parser::caseSenP(int layer, int attr_expType_inh, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer, attr_constType, attr_constVal;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CASE_SEN, true);
    if (symbol.type == TYPE_SYM::CASETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP(&attr_constType, &attr_constVal));
        if (attr_constType != attr_expType_inh)
        {
            addErrorMessage(symbol.line, 'o', "case语句常量类型不一致");
        }
        if (!(symbol.type == TYPE_SYM::COLON)){
            printPos(366134);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER, isFunc, attr_retType_inh, attr_retNum_syn));
    }
    else
    {
        printPos(96911);
    }
    return node;
}

inline SynNode *Parser::caseListP(int layer, int attr_expType_inh, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CASE_LIST, true);
    node->addChild(caseSenP(LAYER, attr_expType_inh, isFunc, attr_retType_inh, attr_retNum_syn));
    while (symbol.type == TYPE_SYM::CASETK)
    {
        node->addChild(caseSenP(LAYER, attr_expType_inh, isFunc, attr_retType_inh, attr_retNum_syn));
    }
    return node;
}

inline SynNode *Parser::defaultP(int layer, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::DEFAULT, true);
    if (symbol.type == TYPE_SYM::DEFAULTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (!(symbol.type == TYPE_SYM::COLON))
        {
            printPos(888888);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER, isFunc, attr_retType_inh, attr_retNum_syn));
    }
    else
    {
        printPos(2227222);
        addErrorMessage(symbol.line, 'p', "default语句缺失");
    }
    return node;
}

inline SynNode *Parser::readSenP()
{
    int LINE;
    string NAME;
    SymTableEntry *sym = nullptr;
    NonTerNode *node = new NonTerNode(TYPE_NTS::READ_SEN, true);
    if (symbol.type == TYPE_SYM::SCANFTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &LINE));
            sym = getEntryByName(NAME);
            if (sym == nullptr)
            {
                addErrorMessage(LINE, 'c', "读语句引用了未定义的名字");
            }
            else if (sym->getCATE() == CAT_CONST)
            {
                addErrorMessage(LINE, 'j', "读语句给常量赋值");
            }
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(214746);
                addErrorMessage(symbol.line, 'l', "读语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
            printPos(77242);
    }
    else
        printPos(998754);
    return node;
}

inline SynNode *Parser::writeSenP(int layer)
{
    int LAYER = layer, LINE;
    string NAME;
    SymTableEntry *sym = nullptr;
    NonTerNode *node = new NonTerNode(TYPE_NTS::WRITE_SEN, true);
    if (symbol.type == TYPE_SYM::PRINTFTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            if (symbol.type == TYPE_SYM::STRCON)
            {
                node->addChild(stringP());
                if (symbol.type == TYPE_SYM::COMMA)
                {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                    node->addChild(expressionP(LAYER));
                }
            }
            else
            {
                node->addChild(expressionP(LAYER));
            }
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(424249);
                addErrorMessage(symbol.line, 'l', "写语句没有右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else
            printPos(892648);
    }
    return node;
}

SynNode *Parser::returnSenP(int layer, int type)
{
    int LAYER = layer, TYPE = TYPE_VOID, LINE = -1;
    NonTerNode *node = new NonTerNode(TYPE_NTS::RETURN_SEN, true);
    LINE = symbol.line;
    if (symbol.type == TYPE_SYM::RETURNTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            TYPE = node->getLastChild()->isCHARconst() ? TYPE_CHAR : TYPE_INT;
            if (!(symbol.type == TYPE_SYM::RPARENT))
            {
                printPos(1984);
                addErrorMessage(symbol.line, 'l', "返回语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
    }
    else
    {
        printPos(99813);
    }
    node->IDEN_TYPE = TYPE;
    if (type == TYPE_VOID && TYPE != type)
    {
        addErrorMessage(LINE, 'g', "无返回值函数有不匹配返回语句");
    }
    else if (type != TYPE_VOID && type != TYPE)
    {
        addErrorMessage(LINE, 'h', "有返回值函数有不匹配返回语句");
    }
    return node;
}

inline SynNode *Parser::sentenceListP(int layer, int type)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SEN_LIST, true);
    while (true)
    {
        if (symbol.type == TYPE_SYM::WHILETK ||
            symbol.type == TYPE_SYM::FORTK ||
            symbol.type == TYPE_SYM::IFTK ||
            symbol.type == TYPE_SYM::SCANFTK ||
            symbol.type == TYPE_SYM::PRINTFTK ||
            symbol.type == TYPE_SYM::SWITCHTK ||
            symbol.type == TYPE_SYM::RETURNTK ||
            symbol.type == TYPE_SYM::SEMICN ||
            symbol.type == TYPE_SYM::IDENFR ||
            symbol.type == TYPE_SYM::LBRACE)
        {
            int ty = symbol.type == TYPE_SYM::RETURNTK ? type : TYPE_VOID;
            node->addChild(sentenceP(LAYER, ty));
        }
        else
            break;
    }
    return node;
}

inline SynNode *Parser::compoundSenP(int layer, int type)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::COMPOUND_SEN, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(constDecP(LAYER));
    }
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(varDecP(LAYER));
    }
    node->addChild(sentenceListP(LAYER + 1, type));
    return node;
}

inline SynNode *Parser::mainP(int layer)
{
    int LAYER = layer;
    NonTerNode *node = new NonTerNode(TYPE_NTS::MAIN, true);
    if (!(symbol.type == TYPE_SYM::VOIDTK))
        printPos(626);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::MAINTK))
        printPos(525);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::LPARENT))
        printPos(556);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::RPARENT))
        printPos(715);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::LBRACE))
        printPos(987);
    node->addChild(new TerNode(symbol));
    nextSym();
    node->addChild(compoundSenP(LAYER));
    if (!(symbol.type == TYPE_SYM::RBRACE))
        printPos(158);
    node->addChild(new TerNode(symbol));
    nextSym();
    return node;
}

SynNode *Parser::parse()
{
    int LAYER = 0;
    NonTerNode *node = new NonTerNode(TYPE_NTS::PROGRAM, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(constDecP(LAYER));
    }
    preReadSym(2);
    if (!(cacheContainsSym(TYPE_SYM::LPARENT)))
    {
        flushPreRead();
        node->addChild(varDecP(LAYER));
    }
    flushPreRead();
    while (true)
    {
        preReadSym(2);
        if (cacheContainsSym(TYPE_SYM::LPARENT) &&
            !cacheContainsSym(TYPE_SYM::MAINTK))
        {
            if (symbol.type == TYPE_SYM::VOIDTK)
            {
                flushPreRead();
                node->addChild(nonrefuncDefineP(LAYER));
            }
            else
            {
                flushPreRead();
                node->addChild(refuncDefineP(LAYER));
            }
        }
        else
        {
            flushPreRead();
            break;
        }
    }
    node->addChild(mainP());
    return node;
}