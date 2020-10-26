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


SynNode *Parser::arrayConstP(int attr_size_inh, int *attr_intType_syn, int*attr_intLine_syn)
{
    int i = 0, attr_temType;
    int attr_conVal_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP( attr_intType_syn, &attr_conVal_syn, attr_intLine_syn));
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
            int attr_line_tem;
            node->addChild(constP(&attr_temType, &attr_conVal_syn, &attr_line_tem));
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

SynNode *Parser::doubleArrayConstP
(int attr_size1_inh, int attr_size2_inh, int *attr_intType_syn, int* attr_intLine_syn)
{
    int i = 0, attr_temType, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::DBARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arrayConstP(attr_size2_inh, attr_intType_syn, &attr_line));
        i++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(attr_size2_inh, &attr_temType, &attr_line));
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

SynNode *Parser::oneDdeclareP(int *attr_size_syn)
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

SynNode* Parser::referenceP(int layer, int* attr_intType_syn, int* attr_value_syn, bool isAssign, int attr_assignVal_inh=0) {
    string attr_strName_syn;
    int attr_intLine_syn, LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::REFERENCE, false);
    node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
    SymTableEntry* attr_sym = getEntrySymByName(attr_strName_syn);

    if (attr_sym == nullptr) {
        addErrorMessage(symbol.line, 'c', "引用了未定义的名字->"+attr_strName_syn);
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
    return node;
}

void Parser::semicnP(NonTerNode* node) {
    if (!(symbol.type == TYPE_SYM::SEMICN))
    {
        addErrorMessage(symbol.line, 'k', "缺少分号");
        printPos(883833);
    }
    else {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
}
