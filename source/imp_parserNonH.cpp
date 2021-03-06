﻿#include "../header/lexical.h"
#include "../header/parser.h"
#include "../header/synTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

/*CLASS: PARSER*/

using namespace std;


SynNode *Parser::arrayConstP(int attr_size_inh, int *attr_intType_syn, int*attr_intLine_syn, vector<int>& iniList)
{
    int i = 0, attr_temType;
    int attr_conVal_syn;
    NonTerNode *node = new NonTerNode(TYPE_NTS::ARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP( attr_intType_syn, &attr_conVal_syn, attr_intLine_syn));
        iniList.push_back(attr_conVal_syn);
        i++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            i++;
            nextSym();
            int attr_line_tem;
            node->addChild(constP(&attr_temType, &attr_conVal_syn, &attr_line_tem));
            iniList.push_back(attr_conVal_syn);
            if (attr_temType != *attr_intType_syn)
            {
                *attr_intType_syn = _TYPE_ERROR;
            }
        }
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(99046);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        *attr_intLine_syn = symbol.line;
        if (i != attr_size_inh)
        {
            printPos(99965);
            addErrorMessage(symbol.line, 'n', "数组第一维初始化个数不匹配");
        }
    }
    else
    {
        printPos(25261);
    }
    return node;
}

SynNode *Parser::doubleArrayConstP
(int attr_size1_inh, int attr_size2_inh, int *attr_intType_syn, int* attr_intLine_syn, vector<int>& iniList)
{
    int i = 0, attr_temType, attr_line;
    NonTerNode *node = new NonTerNode(TYPE_NTS::DBARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
        vector<int> iniSubList;
        node->addChild(arrayConstP(attr_size2_inh, attr_intType_syn, &attr_line, iniSubList));
        iniList.insert(iniList.end(), iniSubList.begin(), iniSubList.end());
        iniSubList.clear();
        i++;
        while (symbol.type == TYPE_SYM::COMMA)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(attr_size2_inh, &attr_temType, &attr_line, iniSubList));
            if (attr_temType != *attr_intType_syn)
            {
                *attr_intType_syn = _TYPE_ERROR;
            }
            iniList.insert(iniList.end(), iniSubList.begin(), iniSubList.end());
            iniSubList.clear();
            i++;
        }
        if (!(symbol.type == TYPE_SYM::RBRACE))
        {
            printPos(84762);
        }
        *attr_intLine_syn = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (i > attr_size1_inh)
        {
            printPos(91651);
            addErrorMessage(symbol.line, 'n', "数组第二维初始化维数不匹配");
        }
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
        int attr_line = symbol.line;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::CHARCON) {
            addErrorMessage(symbol.line, 'i', "数组定义时下标为字符型");
        }
        else {
            node->addChild(unsignedIntP(attr_size_syn));
        }
        if (symbol.type == TYPE_SYM::RBRACK)
        {
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            printPos(36536);
            addErrorMessage(attr_line, 'm', "一维数组定义无右括号");
        }
    }
    else
    {
        printPos(42424);
    }
    return node;
}

SynNode* Parser::referenceP(int* attr_intType_syn, bool isAssign, string& name, 
                            bool* isScaler, string& i, bool* isCon_i, string& j, bool* isCon_j) {
    string attr_strName_syn;
    int attr_intLine_syn;
    NonTerNode* node = new NonTerNode(TYPE_NTS::REFERENCE, false);
    node->addChild(idenP(attr_strName_syn, &attr_intLine_syn));
    SymTableEntry* attr_sym = getEntrySymByName(attr_strName_syn);
    name = attr_strName_syn;
    i = "";
    j = "";
    if (attr_sym == nullptr) {
        addErrorMessage(symbol.line, 'c', "引用了未定义的名字->"+attr_strName_syn);
    }
    else if (isAssign && attr_sym->getCATE() == _CAT_CONST) {
        addErrorMessage(symbol.line, 'j', "改变常量的值");
    } 
    else {
        *attr_intType_syn = attr_sym->getTYPE();
        if (symbol.type == TYPE_SYM::LBRACK) // ARRAY
        {
            *isScaler = false;
            attr_intLine_syn = symbol.line;
            node->addChild(new TerNode(symbol));
            int attr_subValue_syn, attr_subType_syn;
            nextSym();
            string temVar1;
            bool isCon;
            node->addChild(expressionP(&attr_subType_syn, &isCon, temVar1)); 
            i = temVar1;
            *isCon_i = isCon;
            if (attr_subType_syn == _TYPE_CHAR)
            {
                addErrorMessage(symbol.line, 'i', "数组下标为字符型");
            }
            if (symbol.type == TYPE_SYM::RBRACK)
            {
                node->addChild(new TerNode(symbol));
                nextSym();
            }
            else {
                printPos(6516151);
                addErrorMessage(attr_intLine_syn, 'm', "数组元素缺少右中括号");
            }
            if (symbol.type == TYPE_SYM::LBRACK)
            {
                node->addChild(new TerNode(symbol));
                int attr_subValue_syn, attr_subType_syn;
                nextSym();
                string temVar2;
                bool isCon;
                node->addChild(expressionP(&attr_subType_syn, &isCon, temVar2)); 
                j = temVar2;
                *isCon_j = isCon;
                if (attr_subType_syn == _TYPE_CHAR)
                {
                    addErrorMessage(symbol.line, 'i', "数组下标为字符型");
                }
                if (symbol.type == TYPE_SYM::RBRACK)
                {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                }
                else {
                    printPos(881651);
                    addErrorMessage(attr_intLine_syn, 'm', "二维数组元素缺少右中括号");
                }
            }
        }
        else
        {
            *isScaler = true;
            // just var scaler
            i = "";
            j = "";
        }
    }
    return node;
}

void Parser::semicnP(NonTerNode* node) {
    if (symbol.type == TYPE_SYM::SEMICN)
    {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else {
        addErrorMessage(symbol.line - 1, 'k', "缺少分号");
        printPos(883833);
    }
}
