#include "../header/lexical.h"
#include "../header/parser.h"
#include "../header/synTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <tuple>

/*CLASS: PARSER*/

using namespace std;


inline SynNode *Parser::stringP(string& attr_str_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::STRING, true);
    if (symbol.type == TYPE_SYM::STRCON)
    {
        node->addChild(new TerNode(symbol));
        attr_str_syn = symbol.token;
        addString2Global(attr_str_syn);
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

inline SynNode *Parser::constDefP(int* attr_lastLine)
{
    int attr_cate_inh = _CAT_CONST, attr_type_syn = -1, attr_intLine_syn = 0;
    string attr_strName_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEF, true);
    if (symbol.type == TYPE_SYM::INTTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_INT;
        nextSym();
        node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strName_syn))
        {
            addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
        }
        if (!(symbol.type == TYPE_SYM::ASSIGN))
        {
            printPos(414123);
        }
        node->addChild(new TerNode(symbol));
        *attr_lastLine = symbol.line;
        nextSym();
        int attr_int_syn = 0;
        node->addChild(intP(&attr_int_syn));
        ScalerSymEntry* sym = new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn);
        sym->setAssigned();
        sym->value = attr_int_syn;
        addSymbolEntry(sym);
        
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
            if (checkDuplicate(attr_strName_syn))
            {
                addErrorMessage(attr_intLine_syn, 'b', "常量定义时名字重定义");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(97135);
            }
            node->addChild(new TerNode(symbol));
            *attr_lastLine = symbol.line;
            nextSym();
            node->addChild(intP(&attr_int_syn));
            ScalerSymEntry* sym = new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn);
            sym->setAssigned();
            addSymbolEntry(sym);
            sym->value = attr_int_syn;
        }
    }
    else if (this->symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_CHAR;
        nextSym();
        node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strName_syn))
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
        ScalerSymEntry* sym = new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn);
        addSymbolEntry(sym);
        sym->setAssigned();
        sym->value = static_cast<int>(attr_char_syn);
        while (this->symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
            if (checkDuplicate(attr_strName_syn))
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
            ScalerSymEntry* sym = new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn);
            addSymbolEntry(sym);
            sym->setAssigned();
            sym->value = static_cast<int>(attr_char_syn);
        }
    }
    else
    {
        printPos(4123123);
    }
    return node;
}

inline SynNode *Parser::constDecP()
{
    int attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEC, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(&attr_line));
        if (symbol.type == TYPE_SYM::SEMICN)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            addErrorMessage(attr_line, 'k', "常量定义中缺少分号");
            printPos(76262);
        }
    }
    else
    {
        printPos(6381);
    }
    while (this->symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(&attr_line));
        if (this->symbol.type == TYPE_SYM::SEMICN)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            addErrorMessage(attr_line, 'k', "常量定义中缺少分号");
            printPos(51242);
        }
    }
    return node;
}

inline SynNode *Parser::idenP(string& attr_strName_syn, int *attr_intLine_syn)
{ // both default = nullptr
    NonTerNode *node = new NonTerNode(TYPE_NTS::IDEN, false);
    if (!(symbol.type == TYPE_SYM::IDENFR))
    {
        printPos(22);
    }
    attr_strName_syn = symbol.token;
    *attr_intLine_syn = symbol.line;
    node->addChild(new TerNode(symbol));
    nextSym();
    return node;
}

inline SynNode *Parser::decHeadP(string& attr_strNmae_syn, int *attr_intType_syn, int *attr_intLine_syn)
{
    NonTerNode* node = new NonTerNode(TYPE_NTS::DEC_HEAD, true);
    node->addChild(typeIdenP(attr_intType_syn));
    node->addChild(idenP(attr_strNmae_syn, attr_intLine_syn));
    return node;
}

inline SynNode *Parser::constP(int* attr_intTYpe_syn, int* attr_int_syn, int *attr_intLine_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONSTANT, true);
    if (symbol.type == TYPE_SYM::CHARCON)
    {
        *attr_intLine_syn = symbol.line;
        char attr_temChar;
        node->addChild(charP(&attr_temChar));
        *attr_int_syn = static_cast<int>(attr_temChar);
        *attr_intTYpe_syn = _TYPE_CHAR;
    }
    else
    {
        *attr_intLine_syn = symbol.line;
        node->addChild(intP(attr_int_syn));
        *attr_intTYpe_syn = _TYPE_INT;
    }
    return node;
}


inline SynNode *Parser::varDerWithInitP(int attr_intType_inh, int* attr_lastLine_syn)
{
    int attr_cate_inh = _CAT_VAR;
    int attr_size1_syn, attr_size2_syn, attr_intLine_syn = 0;
    int attr_value_syn, attr_conType_syn;
    string attr_strNmae_syn;
    bool hasASSIGN = false;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VARDEF_WITH_INIT, true);
    node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
    if (checkDuplicate(attr_strNmae_syn))
    {
        addErrorMessage(attr_intLine_syn, 'b', "有初始化的变量定义时名字重定义");
    }
    if (symbol.type == TYPE_SYM::ASSIGN)
    {
        hasASSIGN = true;
        node->addChild(new TerNode(symbol));
        nextSym();
        *attr_lastLine_syn = symbol.line;
        node->addChild(constP(&attr_conType_syn, &attr_value_syn, &attr_intLine_syn));
        if (attr_intType_inh != attr_conType_syn)
        {
            addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化标量常量类型不一致");
        }
        SymTableEntry* sym = new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh);
        addSymbolEntry(sym);
        intermediate->addInterCode(INT_OP::ASSIGN, attr_strNmae_syn, attr_intType_inh, int2str(attr_value_syn), attr_conType_syn, true, "", _INV, false);
        sym->setAssigned();
    }
    else if (symbol.type == TYPE_SYM::LBRACK) // ARRAY
    {
        node->addChild(oneDdeclareP(&attr_size1_syn));
        if (symbol.type == TYPE_SYM::ASSIGN)
        {
            hasASSIGN = true;
            node->addChild(new TerNode(symbol));
            nextSym();
            vector<int>* iniList = new vector<int>;
            node->addChild(arrayConstP(attr_size1_syn, &attr_conType_syn, &attr_intLine_syn, *iniList));
            if (attr_intType_inh != attr_conType_syn)
            {
                addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化一维数组常量类型不一致");
            }
            *attr_lastLine_syn = attr_intLine_syn;
            intermediate->addInterCode(INT_OP::ARRINI, attr_strNmae_syn, attr_conType_syn, *iniList);
            addSymbolEntry(
                new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn));
        }
        else if (symbol.type == TYPE_SYM::LBRACK)
        {
            node->addChild(oneDdeclareP(&attr_size2_syn));
            if (symbol.type == TYPE_SYM::ASSIGN)
            {
                hasASSIGN = true;
                node->addChild(new TerNode(symbol));
                nextSym();
                vector<int>* iniList = new vector<int>;
                node->addChild(doubleArrayConstP(attr_size1_syn, attr_size2_syn, &attr_conType_syn, &attr_intLine_syn, *iniList));
                if (attr_intType_inh != attr_conType_syn)
                {
                    addErrorMessage(attr_intLine_syn, 'o', "变量定义初始化二维数组常量类型不一致");
                }
                *attr_lastLine_syn = attr_intLine_syn;
                intermediate->addInterCode(INT_OP::ARRINI, attr_strNmae_syn, attr_conType_syn, *iniList);
                addSymbolEntry(
                    new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn, attr_size2_syn));
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

inline SynNode *Parser::varDerWithoutInitP(int attr_intType_inh, int* attr_lastLine_syn)
{
    //cout << "entered vardefwithoutini" << endl;
    int attr_cate_inh = _CAT_VAR, attr_size1_syn, attr_size2_syn;
    int attr_intLine_syn = 0;
    string attr_strNmae_syn;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEFWIOU_INIT, true);
    node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
    *attr_lastLine_syn = attr_intLine_syn;
    if (checkDuplicate(attr_strNmae_syn))
    {
        addErrorMessage(attr_intLine_syn, 'b', "无初始化变量定义时名字重定义");
    }
    int dim = 0;
    if (symbol.type == TYPE_SYM::LBRACK) // ARRAY
    {
        *attr_lastLine_syn = symbol.line;
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
        addSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh));
    }
    else if (dim == 1)
    {
        addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn));
    }
    else if (dim == 2)
    {
        addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn, attr_size2_syn));
    }
    while (symbol.type == TYPE_SYM::COMMA)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strNmae_syn))
        {
            addErrorMessage(attr_intLine_syn, 'b', "无初始化变量定义时名字重定义");
        }
        *attr_lastLine_syn = attr_intLine_syn;
        dim = 0;
        if (symbol.type == TYPE_SYM::LBRACK)
        {
            *attr_lastLine_syn = symbol.line;
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
            addSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh));
        }
        else if (dim == 1)
        {
            addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn));
        }
        else if (dim == 2)
        {
            addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, attr_size1_syn, attr_size2_syn));
        }
    }
    return node;
}

SynNode *Parser::varDecP()
{
    int attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VAR_DEC, true);
    node->addChild(varDefP(&attr_line));
    if (symbol.type == TYPE_SYM::SEMICN)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else {
        printPos(9146);
        addErrorMessage(attr_line, 'k', "变量说明缺少分号");
    }
    while (true)
    {
        preReadSym(2);
        if (!cacheContainsSym(TYPE_SYM::LPARENT) &&
            (symbol.type == TYPE_SYM::CHARTK ||
             symbol.type == TYPE_SYM::INTTK))
        {
            flushPreRead();
            node->addChild(varDefP(&attr_line));
            if (symbol.type == TYPE_SYM::SEMICN)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(22431);
                addErrorMessage(attr_line, 'k', "变量说明缺少分号");
            }
        }
        else
        {
            flushPreRead();
            break;
        }
    }
    return node;
}

inline SynNode *Parser::varDefP(int* attr_lastLine)
{
    int attr_intType_tem;
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
            node->addChild(varDerWithoutInitP(attr_intType_tem, attr_lastLine));
            break;
        }
        if (cacheContainsSym(TYPE_SYM::ASSIGN))
        {
            flushPreRead();
            node->addChild(varDerWithInitP(attr_intType_tem, attr_lastLine));
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
    else {
        printPos(51749);
    }
    return node;
}

inline SynNode *Parser::termP(int* attr_intType_syn, bool * isCon_syn, string& termVar)
{
    int attr_type_tem;
    bool isCon_tem;
    *attr_intType_syn = _TYPE_CHAR; // set type default as char
    *isCon_syn = true;
    NonTerNode *node = new NonTerNode(TYPE_NTS::TERM, true);
    string temVar;
    node->addChild(factorP(&attr_type_tem, &isCon_tem, temVar));
    if (attr_type_tem == _TYPE_INT)
    {
        *attr_intType_syn = _TYPE_INT;
    }
    if (symbol.type != TYPE_SYM::MULT && symbol.type != TYPE_SYM::DIV) { // only one factor
        termVar = temVar;
        *isCon_syn = isCon_tem;
    }
    else {
        *isCon_syn = false;
        termVar = intermediate->nextTempVar();
        intermediate->addInterCode(INT_OP::ASSIGN, termVar, _TYPE_INT, temVar, attr_type_tem, isCon_tem, "", _INV, false);
        while (symbol.type == TYPE_SYM::MULT || symbol.type == TYPE_SYM::DIV)
        {
            bool isMult = (symbol.type == TYPE_SYM::MULT);
            *attr_intType_syn = _TYPE_INT;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(factorP(&attr_type_tem, &isCon_tem, temVar));
            if (isMult) {
                intermediate->addInterCode(INT_OP::MULT, termVar, _TYPE_INT, termVar, _TYPE_INT, false, temVar, attr_type_tem, isCon_tem);
            }
            else {
                intermediate->addInterCode(INT_OP::DIV, termVar, _TYPE_INT, termVar, _TYPE_INT, false, temVar, attr_type_tem, isCon_tem);
            }
        }
    }
    return node;
}

inline SynNode *Parser::factorP(int* attr_intType_syn, bool* isCon_syn, string& facVar)
{
    int attr_line;
    *isCon_syn = false;
    NonTerNode *node = new NonTerNode(TYPE_NTS::FACTOR, true);
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        attr_line = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP(attr_intType_syn, isCon_syn, facVar));
        *attr_intType_syn = _TYPE_INT;
        if (symbol.type == TYPE_SYM::RPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            printPos(914151);
            addErrorMessage(attr_line, 'l', "因子中缺少右括号");
        }
    }
    else if (symbol.type == TYPE_SYM::CHARCON)
    {
        char attr_char_syn;
        node->addChild(charP(&attr_char_syn));
        facVar = int2str(static_cast<int>(attr_char_syn));
        *attr_intType_syn = _TYPE_CHAR;
        *isCon_syn = true;
    }
    else if (symbol.type == TYPE_SYM::PLUS ||
             symbol.type == TYPE_SYM::MINU || symbol.type == TYPE_SYM::INTCON)
    {
        int attr_int_syn;
        node->addChild(intP(&attr_int_syn));
        facVar = int2str(attr_int_syn);
        *attr_intType_syn = _TYPE_INT;
        *isCon_syn = true;
    }
    else if (symbol.type == TYPE_SYM::IDENFR)
    {
        preReadSym(1);
        if (!cacheContainsSym(TYPE_SYM::LPARENT))
        {
            flushPreRead();
            string idenName;
            bool isScaler;
            string i, j;
            bool ic, jc;
            node->addChild(referenceP(attr_intType_syn, false, idenName, &isScaler, i, &ic, j, &jc));
            // CODE GENE
            if (isScaler) { // just int or char
                facVar = idenName;
            }
            else { // temvar = a[i][j]
                string temVar = intermediate->nextTempVar();
                intermediate->addInterCode(INT_OP::ASSIGN, temVar, *attr_intType_syn, 
                i, _TYPE_INT, ic,
                j, _TYPE_INT, jc, 
                idenName, *attr_intType_syn, false,
                true);
                facVar = temVar;
            }
        }
        else if (cacheContainsSym(TYPE_SYM::LPARENT))
        {
            flushPreRead();
            node->addChild(callFuncSenP(attr_intType_syn, facVar));
        }
    }
    else
        printPos(99517);
    return node;
}

inline SynNode *Parser::expressionP(int* attr_intType_syn, bool* isCon_syn, string& expVar)
{
    int attr_type_tem;
    *isCon_syn = true;
    *attr_intType_syn = _TYPE_CHAR;
    
    NonTerNode *node = new NonTerNode(TYPE_NTS::EXPERSSION, true);
    bool isMinu = (symbol.type == TYPE_SYM::MINU);
    if (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)
    {
        *attr_intType_syn = _TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    string temVar;
    bool isCon_tem;
    node->addChild(termP(&attr_type_tem, &isCon_tem, temVar));
    
    if (attr_type_tem == _TYPE_INT) {
        *attr_intType_syn = _TYPE_INT;
    }
    if (symbol.type != TYPE_SYM::PLUS && symbol.type != TYPE_SYM::MINU) { // only one term
        if (isMinu) {
            expVar = intermediate->nextTempVar();
            *isCon_syn = false;
            intermediate->addInterCode(INT_OP::SUB, expVar, _TYPE_INT, int2str(0), _TYPE_INT, true, temVar, attr_type_tem, isCon_tem);
        }
        else {
            expVar = temVar;
            *isCon_syn = isCon_tem;
        }
    }
    else {
        *isCon_syn = false;
        expVar = intermediate->nextTempVar();  
        if (isMinu) {
            intermediate->addInterCode(INT_OP::SUB, expVar, _TYPE_INT, int2str(0), _TYPE_INT, true, temVar, attr_type_tem, isCon_tem);
        }
        else {
            intermediate->addInterCode(INT_OP::ASSIGN, expVar, _TYPE_INT, temVar, attr_type_tem, isCon_tem, "", _INV, false);
        }
        while (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)
        {
            isMinu = (symbol.type == TYPE_SYM::MINU);
            *attr_intType_syn = _TYPE_INT;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(termP(&attr_type_tem, &isCon_tem, temVar));
            intermediate->addInterCode((isMinu ? INT_OP::SUB : INT_OP::ADD),
                                      expVar, _TYPE_INT, expVar, *attr_intType_syn, false, temVar, attr_type_tem, isCon_tem);

        }
    }
    return node;
}

inline SynNode *Parser::arguListP(int *attr_num_syn, FuncSymEntry *func)
{
    int attr_cate = _CAT_VAR, attr_line = 0, n = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ARGLIST, true);
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK)
    {
        int attr_type_tem;
        node->addChild(typeIdenP(&attr_type_tem));
        node->addChild(idenP(attr_strName, &attr_line));
        int argId = 0;
        // 这里应该不用检查重复
        addSymbolEntry(
            new FormalVarSymEntry(attr_strName, attr_cate, attr_type_tem, func->getINDEX(), argId, func)
        );
        argId++;
        func->addParaType(attr_type_tem);
        func->formalArgNameList.push_back(attr_strName);
        //intermediate->addInterCode(INT_OP::PARA, func->getName(), func->getTYPE(), attr_strName, attr_type_tem, false, "", _INV, false);
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(typeIdenP(&attr_type_tem));
            node->addChild(idenP(attr_strName, &attr_line));
            if (checkDuplicate(attr_strName)) 
            {
                addErrorMessage(attr_line, 'b', "参数列表中重复定义名字");
            }
            addSymbolEntry(
                new FormalVarSymEntry(attr_strName, attr_cate, attr_type_tem, func->getINDEX(), argId, func)
            );
            argId++;
            func->addParaType(attr_type_tem);
            func->formalArgNameList.push_back(attr_strName);
            //intermediate->addInterCode(INT_OP::PARA, func->getName(), func->getTYPE(), attr_strName, attr_type_tem, false, "", _INV, false);
            n++;
        }
    }
    *attr_num_syn = n;
    return node;
}

SynNode *Parser::refuncDefineP()
{
    int attr_type, attr_cate = _CAT_FUNC, attr_line = -1, attr_argnum = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::REFUNC_DEF, true);
    node->addChild(decHeadP(attr_strName, &attr_type, &attr_line));
    if (checkDuplicate(attr_strName))
    {
        addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
    }
    intermediate->addInterCode(INT_OP::FUNC, "", _INV, attr_strName, _INV, false, "", _INV, false);
    FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_cate, attr_type, 0);
    addSymbolEntry(symFUNC);
    envTable.addTable(attr_strName);
    attr_line = symbol.line;
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arguListP(&attr_argnum, symFUNC));
        if (symbol.type == TYPE_SYM::RPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            printPos(123456);
            addErrorMessage(attr_line, 'l', "nonvoid函数定义缺少右括号");
        }
    }
    else
    {
        printPos(22009);
    }
    symFUNC->setARGNUM(attr_argnum);
    int attr_retNum = 0;
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(compoundSenP(true, attr_type, &attr_retNum));
        if (attr_retNum == 0) {
            addErrorMessage(symbol.line, 'h', "有返回值的函数缺少return语句");
        }
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
    envTable.popTable();
    intermediate->addInterCode(INT_OP::ENDFUNC, "", _INV, attr_strName, _INV, false, "", _INV, false);

    return node;
}

SynNode *Parser::nonrefuncDefineP()
{
    int attr_type = _TYPE_VOID, attr_cate = _CAT_FUNC, attr_line = -1, attr_argnum = 0;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::NONREFUNC_DEF, true);
    if (symbol.type == TYPE_SYM::VOIDTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(attr_strName, &attr_line));
        if (checkDuplicate(attr_strName))
        {
            addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
        }
        intermediate->addInterCode(INT_OP::FUNC, "", _INV, attr_strName, _INV, false, "", _INV, false);
        FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_cate, attr_line, 0);
        addSymbolEntry(symFUNC);
        envTable.addTable(attr_strName);
        attr_line = symbol.line;
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arguListP(&attr_argnum, symFUNC));
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(8085);
                addErrorMessage(attr_line, 'l', "void函数定义缺少右括号");
            }
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
            node->addChild(compoundSenP(true, attr_type));
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
        envTable.popTable();
        intermediate->addInterCode(INT_OP::ENDFUNC, "", _INV, attr_strName, _INV, false, "", _INV, false);
    }
    else
    {
        printPos(7522);
    }
    return node;
}

inline SynNode *Parser::callFuncSenP(int* attr_intType_syn, string& temVar)
{
    int attr_line = -1;
    string attr_strName;
    SynNode *iden = idenP(attr_strName, &attr_line);
    NonTerNode *node = nullptr;
    FuncSymEntry *func = getFUNC_CALL(attr_strName);
    if (func == nullptr)
    {
        addErrorMessage(attr_line, 'c', "函数调用了未定义的名字");
        node = new NonTerNode(TYPE_NTS::ERROR, false);
    }
    else if (func->getTYPE() == _TYPE_VOID)
    {
        node = new NonTerNode(TYPE_NTS::CALL_NONREFUNC_SEN, true);
        *attr_intType_syn = _TYPE_VOID;
        temVar = "";
    }
    else
    {
        node = new NonTerNode(TYPE_NTS::CALL_REFUNC_SEN, true);
        *attr_intType_syn = func->getTYPE();
        temVar = intermediate->nextTempVar();
    }
    node->addChild(iden);
    attr_line = symbol.line;
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(valueArgueListP(func));
        if (symbol.type == TYPE_SYM::RPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            printPos(99875);
            addErrorMessage(attr_line, 'l', "函数调用缺少右括号");
        }
    }
    else {
        printPos(88278);
    }
    intermediate->addInterCode(INT_OP::JAL, "", _INV, attr_strName, _INV, false, "", _INV, false);
    intermediate->addInterCode(INT_OP::ENDCALL, "", _INV, attr_strName, func->getTYPE(), false, temVar, _INV, false);
    return node;
}

inline SynNode *Parser::valueArgueListP(FuncSymEntry *func)
{
    int attr_argnum = (func == nullptr? 0 : func->getARGNUM());
    int n = 0, attr_type_tem, attr_intLine = -1, attr_value_tem;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VALUE_ARGLIST, true);
    vector<tuple<string, int, bool>> argNameList;
    if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU ||
        symbol.type == TYPE_SYM::IDENFR ||
        symbol.type == TYPE_SYM::LPARENT ||
        symbol.type == TYPE_SYM::LBRACK ||
        symbol.type == TYPE_SYM::INTCON ||
        symbol.type == TYPE_SYM::CHARCON)
    {
        bool isCon_tem;
        string temVar;
        node->addChild(expressionP(&attr_type_tem, &isCon_tem, temVar));
        if (func == nullptr) {
            // do nothing
            cout << "a null func!" << endl;
        }
        else if (func->getParaTypeList().empty() || n >= attr_argnum)
        {
            addErrorMessage(symbol.line, 'd', "函数调用参数个数不匹配");
        }
        else if (attr_type_tem != func->getParaTypeList()[n])
        {
            addErrorMessage(symbol.line, 'e', "函数调用参数类型不匹配");
        }
        argNameList.push_back(make_tuple(temVar, attr_type_tem, isCon_tem));
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            string temVar;
            node->addChild(expressionP(&attr_type_tem, &isCon_tem, temVar));
            if (func == nullptr) {
                // do nothing
            }
            else if (func->getParaTypeList().empty() || n >= attr_argnum)
            {
                addErrorMessage(symbol.line, 'd', "函数调用参数个数不匹配");
            }
            else if (attr_type_tem != func->getParaTypeList()[n])
            {
                addErrorMessage(symbol.line, 'e', "函数调用参数类型不匹配");
            }
            argNameList.push_back(make_tuple(temVar, attr_type_tem, isCon_tem));
            n++;
        }
    }
    if (attr_argnum != n)
    {
        addErrorMessage(symbol.line, 'd', "函数调用个数不匹配");
    }
    intermediate->addInterCode(INT_OP::BEFCALL, "", _INV, func->name, func->type, false, "", _INV, false);
    int k = 0;
    for (auto iter: argNameList) {
        intermediate->addInterCode(INT_OP::PUSH, func->getName(), func->getTYPE(), get<0>(iter), get<1>(iter), get<2>(iter), int2str(k), _INV, false);
        k++;
    }
    return node;
}

inline SynNode *Parser::assignSenP()
{
    int attr_leftType, attr_rightType, attr_rightValue;
    //SymTableEntry *attr_leftSym;
    string attr_strName;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ASSIGN_SEN, true);
    string tarVar;
    bool isScaler;
    string i, j;
    bool ic, jc;
    node->addChild(referenceP(&attr_leftType, true, tarVar, &isScaler, i, &ic, j, &jc));
    if (symbol.type == TYPE_SYM::ASSIGN)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        string temVar;
        bool isCon;
        node->addChild(expressionP(&attr_rightType, &isCon, temVar));
        if (isScaler) { // tarvar = temvar
            intermediate->addInterCode(INT_OP::ASSIGN, tarVar, attr_leftType, temVar, attr_rightType, isCon, "", _INV, false);
        }
        else { // tarvar[i][j] = temvar
            intermediate->addInterCode(INT_OP::ASSIGN, tarVar, attr_leftType, 
                                       i, _TYPE_INT, ic, 
                                       j, _TYPE_INT, jc, 
                                       temVar, attr_rightType, isCon,
                                       false);
        }
    }
    else
    {
        printPos(62562);
    }
    return node;
}

inline SynNode *Parser::ifelseSenP(bool inFunc, string lastLABEL, int attr_retType,  int* attr_retNum_syn)
{
    /*
        if (cd) { if cd not true, jump to label_of_this
            ...
            jump to label_of_end
        } 
        label_of_this:
        else ...
        ...
        label_of_end:
    */
    int attr_intLine = -1;
    bool attr_res;
    string label_of_this = "";
    NonTerNode *node = new NonTerNode(TYPE_NTS::IFELSE_SEN, true);
    if (symbol.type == TYPE_SYM::IFTK)
    {
        label_of_this = intermediate->nextLabel(getCurFuncName());
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            attr_intLine = symbol.line;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(label_of_this)); // add condition for jump to label (OF THIS)
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(5662325);
                addErrorMessage(attr_intLine, 'l', "ifelse中缺失右括号");
            }
        }
        else
            printPos(66725);
    }
    else
    {
        printPos(9947252);
    }
    string t;
    node->addChild(sentenceP(inFunc, t, attr_retType, attr_retNum_syn));
    intermediate->addInterCode(INT_OP::J, "", _INV, lastLABEL, _INV, false, "", _INV, false); // add jump to label (LAST LABEL)
    intermediate->addInterCode(INT_OP::LABEL, "", _INV, label_of_this, _INV, false, "", _INV, false); //add label (OF THIS) for end 

    if (symbol.type == TYPE_SYM::ELSETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(inFunc, lastLABEL, attr_retType, attr_retNum_syn)); // add label (LAST LABEL)
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

inline SynNode *Parser::conditionP(string jumpLabel)
{
    int attr_leftType, attr_rightType, attr_leftVal, attr_rightVal, attr_op;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONDITION, true);
    string temVar1;
    bool isConL, isConR;
    node->addChild(expressionP(&attr_leftType, &isConL, temVar1));
    if (attr_leftType == _TYPE_CHAR)
    {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    node->addChild(compareOpP(&attr_op));
    string temVar2;
    node->addChild(expressionP(&attr_rightType, &isConR, temVar2));
    if (attr_rightType == _TYPE_CHAR)
    {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    // 0 <; 1 <=; 2 >; 3 >=; 4 ==; 5 !=
    // TODO 
    switch (attr_op)
    {
    case 0: 
        intermediate->addInterCode(INT_OP::BGE, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);        
        break;
    case 1:
        intermediate->addInterCode(INT_OP::BGT, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);
        break;
    case 2:
        intermediate->addInterCode(INT_OP::BLE, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);
        break;
    case 3:
        intermediate->addInterCode(INT_OP::BLT, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);
        break;
    case 4:
        intermediate->addInterCode(INT_OP::BNE, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);
        break;
    case 5:
        intermediate->addInterCode(INT_OP::BEQ, jumpLabel, _INV, temVar1, attr_leftType, isConL, temVar2, attr_rightType, isConR);
        break;
    default:
        cout << "no such compare op in conditionP() !";
        break;
    }
    return node;
}

inline SynNode *Parser::stepLengthP(int* attr_len)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::STEP_LEN, true);
    node->addChild(unsignedIntP(attr_len));
    return node;
}

inline SynNode *Parser::loopSenP(bool isFunc, int attr_type_inh, int* attr_retNum_syn)
{
    int attr_line;
    string lastLabel = "", beginLabel = "";
    NonTerNode *node = new NonTerNode(TYPE_NTS::LOOP_SEN, true);
    if (symbol.type == TYPE_SYM::WHILETK)
    {
        beginLabel = intermediate->nextLabel(getCurFuncName(), "WHILEBG");
        lastLabel = intermediate->nextLabel(getCurFuncName(), "WHILEED");
        intermediate->addInterCode(INT_OP::LABEL, "", _INV, beginLabel, _INV, false, "", _INV, false);
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            attr_line = symbol.line;
            bool attr_res;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(lastLabel));
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(11451);
                addErrorMessage(attr_line, 'l', "while语句缺少右括号");
            }
        }
        else{
            printPos(414151);
        }
        node->addChild(sentenceP(isFunc, "", attr_type_inh, attr_retNum_syn));
        intermediate->addInterCode(INT_OP::J, "", _INV, beginLabel, _INV, false, "", _INV, false);
        intermediate->addInterCode(INT_OP::LABEL, "", _INV, lastLabel, _INV, false, "", _INV, false);
    }
    else if (symbol.type == TYPE_SYM::FORTK)
    {
        string attr_strName, temVar, temVar2;
        int attr_line, attr_intType, attr_val, attr_intType2, attr_len;
        bool isMinu;
        node->addChild(new TerNode(symbol));
        nextSym();
        lastLabel = intermediate->nextLabel(getCurFuncName(), "FOR");
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            // phase 1
            
            bool isScaler;
            string i, j;
            bool ic, jc;
            node->addChild(referenceP(&attr_intType, true, temVar, &isScaler, i, &ic, j, &jc));
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(841656);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            int attr_expVal, attr_expType;
            bool isCon;
            node->addChild(expressionP(&attr_expType, &isCon, temVar2));
            if (symbol.type == TYPE_SYM::SEMICN)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号1");
            }
            intermediate->addInterCode(INT_OP::ASSIGN, temVar, attr_intType, temVar2, attr_expType, isCon, "", _INV, false);
            // phase 2
            beginLabel = intermediate->nextLabel(getCurFuncName(), "FORBG");
            intermediate->addInterCode(INT_OP::LABEL, "", _INV, beginLabel, _INV, false, "", _INV, false);
            bool attr_res;
            node->addChild(conditionP(lastLabel));
            if (symbol.type == TYPE_SYM::SEMICN)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号2");
            }
            // phase 3
            node->addChild(referenceP(&attr_intType, true, temVar, &isScaler, i, &ic, j, &jc));
            if (!(symbol.type == TYPE_SYM::ASSIGN))
            {
                printPos(61319);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            
            node->addChild(referenceP(&attr_intType2, true, temVar2, &isScaler, i, &ic, j, &jc));
            isMinu = symbol.type == TYPE_SYM::MINU;
            if (!(symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU))
            {
                printPos(26262);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            attr_line = symbol.line;
            
            node->addChild(stepLengthP(&attr_len));
            
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(911451);
                addErrorMessage(attr_line, 'l', "for语句缺少右括号");
            }
        }
        else
        {
            printPos(907691);
        }
        node->addChild(sentenceP(isFunc, "", attr_type_inh, attr_retNum_syn));
        if (isMinu) {
            intermediate->addInterCode(INT_OP::SUB, temVar, attr_intType, temVar2, attr_intType2, false, int2str(attr_len), _TYPE_INT, true);
        }
        else {
            intermediate->addInterCode(INT_OP::ADD, temVar, attr_intType, temVar2, attr_intType2, false, int2str(attr_len), _TYPE_INT, true);
        }
        intermediate->addInterCode(INT_OP::J, "", _INV, beginLabel, _INV, false, "", _INV, false);
        intermediate->addInterCode(INT_OP::LABEL, "", _INV, lastLabel, _INV, false, "", _INV, false);
    }
    return node;
}

inline SynNode *Parser::switchSenP(bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int attr_expType, attr_val, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SWITCH_SEN, true);
    if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        string lastLabel = intermediate->nextLabel(getCurFuncName(), "SWITCHED");
        string temVar;
        bool isCon;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            attr_line = symbol.line;
            node->addChild(expressionP(&attr_expType, &isCon, temVar));
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(77251);
                addErrorMessage(attr_line, 'l', "switch缺少右括号");
            }
        }
        else
        {
            printPos(14149);
        }
        if (symbol.type == TYPE_SYM::LBRACE)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(caseListP(attr_expType, isFunc, isCon, temVar, lastLabel, attr_retType_inh, attr_retNum_syn));
            node->addChild(defaultP(isFunc, attr_retType_inh, attr_retNum_syn));
            if (!(symbol.type == TYPE_SYM::RBRACE))
            {
                printPos(214156);
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            intermediate->addInterCode(INT_OP::LABEL, "", _INV, lastLabel, _INV, false, "", _INV, false);
        }
        else
        {
            printPos(991737);
        }
    }
    else
    {
        printPos(626666);
    }

    return node;
}

inline SynNode *Parser::caseSenP(int attr_expType_inh, bool isFunc, bool isConExp, string exp_var, string lastLabel, int attr_retType_inh, int* attr_retNum_syn)
{
    int attr_constType, attr_constVal, attr_line;
    string this_label = intermediate->nextLabel(getCurFuncName(), "CASE");
    NonTerNode *node = new NonTerNode(TYPE_NTS::CASE_SEN, true);
    if (symbol.type == TYPE_SYM::CASETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP(&attr_constType, &attr_constVal, &attr_line));
        if (attr_constType != attr_expType_inh)
        {
            addErrorMessage(attr_line, 'o', "case语句常量类型不一致");
        }
        if (!(symbol.type == TYPE_SYM::COLON)){
            printPos(366134);
        }
        intermediate->addInterCode(INT_OP::BNE, this_label, _INV, 
                                   exp_var, attr_expType_inh, isConExp, 
                                   int2str(attr_constVal), attr_constType, true);

        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(isFunc, "", attr_retType_inh, attr_retNum_syn));
        intermediate->addInterCode(INT_OP::J, "", _INV, lastLabel, _INV, false, "", _INV, false);
        intermediate->addInterCode(INT_OP::LABEL, "", _INV, this_label, _INV, false, "", _INV, false);
    }
    else
    {
        printPos(96911);
    }
    return node;
}

inline SynNode *Parser::caseListP(int attr_expType_inh, bool isFunc, 
                                  bool isConExp, string exp_var, string lastLabel, 
                                  int attr_retType_inh, int* attr_retNum_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::CASE_LIST, true);
    node->addChild(caseSenP(attr_expType_inh, isFunc, isConExp, exp_var, lastLabel, attr_retType_inh, attr_retNum_syn));
    while (symbol.type == TYPE_SYM::CASETK)
    {
        node->addChild(caseSenP(attr_expType_inh, isFunc, isConExp, exp_var, lastLabel, attr_retType_inh, attr_retNum_syn));
    }
    return node;
}

inline SynNode *Parser::defaultP(bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
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
        node->addChild(sentenceP(isFunc, "", attr_retType_inh, attr_retNum_syn));
        //popCurTable();
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
    int attr_line/*, LAYER = layer, attr_type, attr_val*/;
    string attr_strName;
    int attr_type;
    bool isScaler;
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
            string i, j;
            bool ic, jc;
            node->addChild(referenceP(&attr_type, true, attr_strName, &isScaler, i, &ic, j, &jc));
            #ifdef PRINT_ERROR_MESSAGE
            if (!isScaler) {
                cout << "read to an array!";
            }
            #endif
            intermediate->addInterCode(INT_OP::SCAN, attr_strName, attr_type, "", _INV, false, "", _INV, false);
            sym = getEntrySymByName(attr_strName);
            if (sym == nullptr)
            {
                addErrorMessage(attr_line, 'c', "读语句引用了未定义的名字");
            }
            else if (sym->getCATE() == _CAT_CONST)
            {
                addErrorMessage(attr_line, 'j', "读语句给常量赋值");
            }
            else if (dynamic_cast<ArraySymEntry*>(sym) != nullptr) {
                printPos(91834); // 读语句的标识符不能是数组
            }
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(214746);
                addErrorMessage(attr_line, 'l', "读语句缺少右括号");
            }
        }
        else
        {
            printPos(77242);
        }
    }
    else
    {
        printPos(998754);
    }
    return node;
}

inline SynNode *Parser::writeSenP()
{
    int attr_line;
    string attr_str = "";
    int attr_type;
    //SymTableEntry *sym = nullptr;
    NonTerNode *node = new NonTerNode(TYPE_NTS::WRITE_SEN, true);
    if (symbol.type == TYPE_SYM::PRINTFTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            attr_line = symbol.line;
            node->addChild(new TerNode(symbol));
            nextSym();
            if (symbol.type == TYPE_SYM::STRCON)
            {
                node->addChild(stringP(attr_str));
                if (symbol.type == TYPE_SYM::COMMA)
                {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                    string temVar;
                    bool isCon;
                    node->addChild(expressionP(&attr_type, &isCon, temVar));
                    intermediate->addInterCode(INT_OP::PRINT, "", _INV, attr_str, _TYPE_STR, true, temVar, attr_type, isCon);
                }
                else {
                    intermediate->addInterCode(INT_OP::PRINT, "", _INV, attr_str, _TYPE_STR, true, "", _INV, false);
                }
            }
            else
            {
                string temVar;
                bool isCon;
                node->addChild(expressionP(&attr_type, &isCon, temVar));
                intermediate->addInterCode(INT_OP::PRINT, "", _INV, "", _INV, false, temVar, attr_type, isCon);
            }
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(424249);
                addErrorMessage(attr_line, 'l', "写语句没有右括号");
            }
        }
        else
        {
            printPos(892648);
        }
    }
    else {
        printPos(991653);
    }
    return node;
}

SynNode *Parser::returnSenP(int attr_retType_inh, int* attr_retNum_syn, bool inMain)
{
    int attr_line = -1;
    int attr_expType = _TYPE_VOID;
    NonTerNode *node = new NonTerNode(TYPE_NTS::RETURN_SEN, true);
    attr_line = symbol.line;
    if (symbol.type == TYPE_SYM::RETURNTK)
    {
        attr_line = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            string temVar;
            bool isCon;
            node->addChild(expressionP(&attr_expType, &isCon, temVar));
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
                intermediate->addInterCode(INT_OP::RETURN, getCurFuncName(), _INV, temVar, attr_expType, isCon, "", _INV, false);
            }
            else {
                printPos(1984);
                addErrorMessage(attr_line, 'l', "返回语句缺少右括号");
            }
        }
        else { // void return
            intermediate->addInterCode(INT_OP::RETURN, getCurFuncName(), _INV, "", _INV, false, "", _INV, false);
        }
        if (inMain) {
            // pass
        }
        else {
            // do some things
        }
    }
    else
    {
        printPos(99813);
    }

    if (attr_retType_inh == _TYPE_VOID && attr_expType != _TYPE_VOID)
    {
        addErrorMessage(attr_line, 'g', "无返回值函数有不匹配返回语句");
    }
    else if (attr_retType_inh != _TYPE_VOID && attr_expType != attr_retType_inh)
    {
        addErrorMessage(attr_line, 'h', "有返回值函数有不匹配返回语句");
        if (attr_retNum_syn) {
            *attr_retNum_syn += 1;
        }
    }
    else {
        if (attr_retNum_syn) {
            *attr_retNum_syn += 1;
        }
    }
    return node;
}


inline SynNode *Parser::sentenceP(bool inFunc, string lastLABEL, int attr_type_inh, int* attr_retNum_syn, bool inMain)
{ 
    int attr_intLine = -1;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SENTENCE, true);
    if (symbol.type == TYPE_SYM::WHILETK || symbol.type == TYPE_SYM::FORTK)
    {
        node->addChild(loopSenP(inFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::IFTK)
    {
        bool fatherIf = false;
        if (lastLABEL == "") {
            fatherIf = true;
            lastLABEL = intermediate->nextLabel(getCurFuncName(), "END");
        }
        else {
            // in if-else chain
        }
        node->addChild(ifelseSenP(inFunc, lastLABEL, attr_type_inh, attr_retNum_syn));
        if (fatherIf) { // add last label
            intermediate->addInterCode(INT_OP::LABEL, "", _INV, lastLABEL, _INV, false, "", _INV, false);
        }
    }
    else if (symbol.type == TYPE_SYM::SCANFTK)
    {
        node->addChild(readSenP());
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::PRINTFTK)
    {
        node->addChild(writeSenP());
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        node->addChild(switchSenP(inFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::RETURNTK)
    {
        node->addChild(returnSenP(attr_type_inh, attr_retNum_syn, inMain));
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::IDENFR)
    {
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::LPARENT))
        {
            int attr_type, attr_val;
            flushPreRead();
            string temVar;
            node->addChild(callFuncSenP(&attr_type, temVar));
        }
        else
        {
            flushPreRead();
            node->addChild(assignSenP());
        }
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceListP(inFunc, attr_type_inh, attr_retNum_syn));
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(737892);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else
    { // 空语句
        semicnP(node);
    }
    return node;
}


inline SynNode *Parser::sentenceListP(bool inFunc, int attr_type_inh, int* attr_retNum_syn, bool inMain)
{
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
            node->addChild(sentenceP(inFunc, "", attr_type_inh, attr_retNum_syn, inMain));
        }
        else{
            break;
        }
    }
    return node;
}

inline SynNode *Parser::compoundSenP(bool inFunc, int attr_type_inh, int* attr_retNum_syn, bool inMain){
    NonTerNode *node = new NonTerNode(TYPE_NTS::COMPOUND_SEN, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(constDecP());
    }
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(varDecP());
    }
    node->addChild(sentenceListP(inFunc, attr_type_inh, attr_retNum_syn));
    return node;
}

inline SynNode *Parser::mainP()
{
    envTable.addTable("main");
    int attr_line = 0;
    NonTerNode *node = new NonTerNode(TYPE_NTS::MAIN, true);
    if (!(symbol.type == TYPE_SYM::VOIDTK))
        printPos(626);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::MAINTK))
        printPos(525);
    node->addChild(new TerNode(symbol));
    nextSym();
    intermediate->addInterCode(INT_OP::FUNC, "", _INV, "main", _INV, false, "", _INV, false);
    if (symbol.type == TYPE_SYM::LPARENT) {
        attr_line = symbol.line;
    }
    else {
        printPos(556);
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    if (symbol.type == TYPE_SYM::RPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else {
        printPos(715);
        addErrorMessage(attr_line, 'l', "主函数缺少右括号");
    }
    if (!(symbol.type == TYPE_SYM::LBRACE))
    {
        printPos(987);
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    int retNum = 0;
    node->addChild(compoundSenP(true, _TYPE_VOID, &retNum));
    intermediate->addInterCode(INT_OP::EXIT, "", _INV, "", _INV, false, "", _INV, false);
    if (!(symbol.type == TYPE_SYM::RBRACE))
    {
        printPos(158);
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    envTable.popTable();
    intermediate->addInterCode(INT_OP::ENDFUNC, "", _INV, "main", _INV, false, "", _INV, false);
    return node;
}

SynNode *Parser::parse()
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::PROGRAM, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(constDecP());
    }
    preReadSym(2);
    if (!(cacheContainsSym(TYPE_SYM::LPARENT)))
    {
        flushPreRead();
        node->addChild(varDecP());
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
                node->addChild(nonrefuncDefineP());
            }
            else
            {
                flushPreRead();
                node->addChild(refuncDefineP());
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