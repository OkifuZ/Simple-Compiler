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


inline SynNode *Parser::stringP(string& attr_str_syn)
{
    NonTerNode *node = new NonTerNode(TYPE_NTS::STRING, true);
    if (symbol.type == TYPE_SYM::STRCON)
    {
        node->addChild(new TerNode(symbol));
        attr_str_syn = symbol.token;
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

inline SynNode *Parser::constDefP(int layer, int* attr_lastLine)
{
    int attr_cate_inh = _CAT_CONST, attr_type_syn = -1, LAYER = layer, attr_intLine_syn = 0;
    string attr_strName_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEF, true);
    if (symbol.type == TYPE_SYM::INTTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_INT;
        nextSym();
        node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strName_syn, LAYER))
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
        addSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
            if (checkDuplicate(attr_strName_syn, LAYER))
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
            addSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        }
    }
    else if (this->symbol.type == TYPE_SYM::CHARTK)
    {
        node->addChild(new TerNode(symbol));
        attr_type_syn = _TYPE_CHAR;
        nextSym();
        node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
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
        addSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
        while (this->symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
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
            addSymbolEntry(new ScalerSymEntry(attr_strName_syn, attr_cate_inh, attr_type_syn, LAYER));
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
    int attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::CONST_DEC, true);
    if (symbol.type == TYPE_SYM::CONSTTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(LAYER, &attr_line));
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
        node->addChild(constDefP(LAYER, &attr_line));
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


inline SynNode *Parser::varDerWithInitP(int layer, int attr_intType_inh, int* attr_lastLine_syn)
{
    int attr_cate_inh = _CAT_VAR, LAYER = layer;
    int attr_size1_syn, attr_size2_syn, attr_intLine_syn = 0;
    int attr_value_syn, attr_conType_syn;
    string attr_strNmae_syn;
    bool hasASSIGN = false;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VARDEF_WITH_INIT, true);
    node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
    if (checkDuplicate(attr_strNmae_syn, LAYER))
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
        addSymbolEntry(
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
            *attr_lastLine_syn = attr_intLine_syn;
            addSymbolEntry(
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
                *attr_lastLine_syn = attr_intLine_syn;
                addSymbolEntry(
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

inline SynNode *Parser::varDerWithoutInitP(int layer, int attr_intType_inh, int* attr_lastLine_syn)
{
    //cout << "entered vardefwithoutini" << endl;
    int attr_cate_inh = _CAT_VAR, LAYER = layer, attr_size1_syn, attr_size2_syn;
    int attr_intLine_syn = 0;
    string attr_strNmae_syn;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEFWIOU_INIT, true);
    node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
    *attr_lastLine_syn = attr_intLine_syn;
    if (checkDuplicate(attr_strNmae_syn, LAYER))
    {
        addErrorMessage(attr_intLine_syn, 'b', "无初始化变量定义时名字重定义");
    }
    int dim = 0;
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
        addSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER));
    }
    else if (dim == 1)
    {
        addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn));
    }
    else if (dim == 2)
    {
        addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn, attr_size2_syn));
    }
    while (symbol.type == TYPE_SYM::COMMA)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(attr_strNmae_syn, &attr_intLine_syn));
        if (checkDuplicate(attr_strNmae_syn, LAYER))
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
            addSymbolEntry(new ScalerSymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER));
        }
        else if (dim == 1)
        {
            addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn));
        }
        else if (dim == 2)
        {
            addSymbolEntry(new ArraySymEntry(attr_strNmae_syn, attr_cate_inh, attr_intType_inh, LAYER, attr_size1_syn, attr_size2_syn));
        }
    }
    return node;
}

SynNode *Parser::varDecP(int layer)
{
    int LAYER = layer;
    int attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::VAR_DEC, true);
    node->addChild(varDefP(LAYER, &attr_line));
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
            node->addChild(varDefP(LAYER, &attr_line));
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

inline SynNode *Parser::varDefP(int layer, int* attr_lastLine)
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
            node->addChild(varDerWithoutInitP(LAYER, attr_intType_tem, attr_lastLine));
            break;
        }
        if (cacheContainsSym(TYPE_SYM::ASSIGN))
        {
            flushPreRead();
            node->addChild(varDerWithInitP(LAYER, attr_intType_tem, attr_lastLine));
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
        /*attr_value_ans = (symbol.type == TYPE_SYM::MULT ? 
            attr_value_ans * attr_value_tem : attr_value_ans / (attr_value_tem == 0 ? 0.1 : attr_value_tem));*/
    }
    /**attr_value_syn = attr_value_ans;*/
    *attr_value_syn = 0;
    return node;
}

inline SynNode *Parser::factorP(int layer, int* attr_intType_syn, int* attr_value_syn)
{
    int LAYER = layer, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::FACTOR, true);
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        attr_line = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP(LAYER, attr_intType_syn, attr_value_syn));
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
        *attr_value_syn = static_cast<int>(attr_char_syn);
        *attr_intType_syn = _TYPE_CHAR;
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
        node->addChild(idenP(attr_strName, &attr_line));
        // 这里应该不用检查重复
        addSymbolEntry(
            new FormalVarSymEntry(attr_strName, attr_cate, attr_type_tem, LAYER, func->getINDEX()));
        func->addParaType(attr_type_tem);
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(typeIdenP(&attr_type_tem));
            node->addChild(idenP(attr_strName, &attr_line));
            if (checkDuplicate(attr_strName, LAYER)) 
            {
                addErrorMessage(attr_line, 'b', "参数列表中重复定义名字");
            }
            addSymbolEntry(
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
    node->addChild(decHeadP(attr_strName, &attr_type, &attr_line));
    if (checkDuplicate(attr_strName, LAYER))
    {
        addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
    }
    FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_cate, attr_type, LAYER, 0);
    addSymbolEntry(symFUNC);
    attr_line = symbol.line;
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arguListP(LAYER + 1, &attr_argnum, symFUNC));
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
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        int attr_retNum = 0;
        node->addChild(compoundSenP(LAYER + 1, true, attr_type, &attr_retNum));
        if (attr_retNum == 0) {
            addErrorMessage(symbol.line, 'h', "有返回值的函数缺少return语句");
        }
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(891947);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        popSym_CurLayer(LAYER + 1);
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
        node->addChild(idenP(attr_strName, &attr_line));
        if (checkDuplicate(attr_strName, LAYER))
        {
            addErrorMessage(attr_line, 'b', "有返回值函数名字重复定义");
        }
        FuncSymEntry *symFUNC = new FuncSymEntry(attr_strName, attr_cate, attr_line, LAYER, 0);
        addSymbolEntry(symFUNC);
        attr_line = symbol.line;
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arguListP(LAYER + 1, &attr_argnum, symFUNC));
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
            node->addChild(compoundSenP(LAYER + 1, true, attr_type));
            if (!(symbol.type == TYPE_SYM::RBRACE))
            {
                printPos(9976);
            }
            node->addChild(new TerNode(symbol));
            popSym_CurLayer(LAYER + 1);
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
    }
    else
    {
        node = new NonTerNode(TYPE_NTS::CALL_REFUNC_SEN, true);
        *attr_intType_syn = func->getTYPE();
    }
    node->addChild(iden);
    attr_line = symbol.line;
    if (symbol.type == TYPE_SYM::LPARENT)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(valueArgueListP(LAYER, func));
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
    
    *attr_value_syn = 0; // so far no compute enabled
    return node;
}

inline SynNode *Parser::valueArgueListP(int layer, FuncSymEntry *func)
{
    int LAYER = layer, attr_argnum = (func == nullptr? 0 : func->getARGNUM());
    int n = 0, attr_type_tem, attr_intLine = -1, attr_value_tem;
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
        n++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER, &attr_type_tem, &attr_value_tem));
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
            n++;
        }
    }
    if (attr_argnum != n)
    {
        addErrorMessage(symbol.line, 'd', "函数调用个数不匹配");
    }
    return node;
}

inline SynNode *Parser::assignSenP(int layer)
{
    int LAYER = layer, attr_leftType, attr_rightType, attr_rightValue;
    //SymTableEntry *attr_leftSym;
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
            attr_intLine = symbol.line;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER, &attr_res));
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
    int LAYER = layer, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::LOOP_SEN, true);
    if (symbol.type == TYPE_SYM::WHILETK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            attr_line = symbol.line;
            bool attr_res;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER, &attr_res));
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
        node->addChild(sentenceP(LAYER, isFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::FORTK)
    {
        string attr_strName;
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
            if (symbol.type == TYPE_SYM::SEMICN)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号1");
            }
            // phase 2
            bool attr_res;
            node->addChild(conditionP(LAYER, &attr_res));
            if (symbol.type == TYPE_SYM::SEMICN)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号2");
            }
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
            attr_line = symbol.line;
            int attr_len;
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
        node->addChild(sentenceP(LAYER, isFunc, attr_type_inh, attr_retNum_syn));
    }
    return node;
}

inline SynNode *Parser::switchSenP(int layer, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer, attr_expType, attr_val, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SWITCH_SEN, true);
    if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            attr_line = symbol.line;
            node->addChild(expressionP(LAYER, &attr_expType, &attr_val));
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
            LAYER = LAYER + 1;
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

inline SynNode *Parser::caseSenP(int layer, int attr_expType_inh, bool isFunc, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer, attr_constType, attr_constVal, attr_line;
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
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER + 1, isFunc, attr_retType_inh, attr_retNum_syn));
        popSym_CurLayer(LAYER + 1);
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
        node->addChild(sentenceP(LAYER + 1, isFunc, attr_retType_inh, attr_retNum_syn));
        popSym_CurLayer(LAYER);
    }
    else
    {
        printPos(2227222);
        addErrorMessage(symbol.line, 'p', "default语句缺失");
    }
    return node;
}

inline SynNode *Parser::readSenP(int layer)
{
    int attr_line/*, LAYER = layer, attr_type, attr_val*/;
    string attr_strName;
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
            node->addChild(idenP(attr_strName, &attr_line));
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

inline SynNode *Parser::writeSenP(int layer)
{
    int LAYER = layer, attr_line;
    string attr_str;
    int attr_type, attr_val;
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
                    node->addChild(expressionP(LAYER, &attr_type, &attr_val));
                }
            }
            else
            {
                node->addChild(expressionP(LAYER, &attr_type, &attr_val));
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
    return node;
}

SynNode *Parser::returnSenP(int layer, int attr_retType_inh, int* attr_retNum_syn)
{
    int LAYER = layer, attr_line = -1;
    int attr_expType = _TYPE_VOID, attr_expVal;
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
            
            node->addChild(expressionP(LAYER, &attr_expType, &attr_expVal));
            if (symbol.type == TYPE_SYM::RPARENT)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(1984);
                addErrorMessage(attr_line, 'l', "返回语句缺少右括号");
            }
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


inline SynNode *Parser::sentenceP(int layer, bool inFunc, int attr_type_inh, int* attr_retNum_syn)
{ 
    int LAYER = layer, attr_intLine = -1;
    NonTerNode *node = new NonTerNode(TYPE_NTS::SENTENCE, true);
    if (symbol.type == TYPE_SYM::WHILETK || symbol.type == TYPE_SYM::FORTK)
    {
        node->addChild(loopSenP(LAYER + 1, inFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::IFTK)
    {
        node->addChild(ifelseSenP(LAYER + 1, inFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::SCANFTK)
    {
        node->addChild(readSenP(LAYER));
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::PRINTFTK)
    {
        node->addChild(writeSenP(LAYER));
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::SWITCHTK)
    {
        node->addChild(switchSenP(LAYER + 1, inFunc, attr_type_inh, attr_retNum_syn));
    }
    else if (symbol.type == TYPE_SYM::RETURNTK)
    {
        node->addChild(returnSenP(LAYER, attr_type_inh, attr_retNum_syn));
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::IDENFR)
    {
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::LPARENT))
        {
            int attr_type, attr_val;
            flushPreRead();
            node->addChild(callFuncSenP(LAYER, &attr_type, &attr_val));
        }
        else
        {
            flushPreRead();
            node->addChild(assignSenP(LAYER));
        }
        semicnP(node);
    }
    else if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceListP(LAYER + 1, inFunc, attr_type_inh, attr_retNum_syn));
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


inline SynNode *Parser::sentenceListP(int layer, bool inFunc, int attr_type_inh, int* attr_retNum_syn)
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
            node->addChild(sentenceP(LAYER, inFunc, attr_type_inh, attr_retNum_syn));
        }
        else{
            break;
        }
    }
    return node;
}

inline SynNode *Parser::compoundSenP(int layer, bool inFunc, int attr_type_inh, int* attr_retNum_syn)
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
    node->addChild(sentenceListP(LAYER, inFunc, attr_type_inh, attr_retNum_syn));
    return node;
}

inline SynNode *Parser::mainP(int layer)
{
    int LAYER = layer, attr_line = 0;
    NonTerNode *node = new NonTerNode(TYPE_NTS::MAIN, true);
    if (!(symbol.type == TYPE_SYM::VOIDTK))
        printPos(626);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::MAINTK))
        printPos(525);
    node->addChild(new TerNode(symbol));
    nextSym();
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
    node->addChild(compoundSenP(LAYER + 1, true, _TYPE_VOID, &retNum));
    if (!(symbol.type == TYPE_SYM::RBRACE))
    {
        printPos(158);
    }
    node->addChild(new TerNode(symbol));
    nextSym();
    popSym_CurLayer(LAYER + 1);
    
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
    node->addChild(mainP(LAYER));
    return node;
}