#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../header/lexical.h"
#include "../header/parser.h"
#include "../header/synTree.h"

using namespace std;

const std::vector<std::string> STRING_NTS_LIST = {
	"<字符串>", "<无符号整数>", "<整数>",
	"<常量定义>", "<常量说明>", "<声明头部>",
	"<常量>", "<变量定义及初始化>", "<变量定义无初始化>", "<变量说明>",
	"<变量定义>", "<参数表>", "<项>",
	"<因子>", "<表达式>", "<语句>",
	"<有返回值函数调用语句>", "<无返回值函数调用语句>",
	"<有返回值函数定义>", "<无返回值函数定义>", "<值参数表>", "<赋值语句>",
	"<条件语句>", "<条件>", "<步长>", "<循环语句>",
	"<情况子语句>", "<情况表>", "<情况语句>", "<缺省>",
	"<写语句>", "<读语句>", "<返回语句>", "<语句列>",
	"<复合语句>",
	"<主函数>", "<程序>"
};

const string get_STRING_NTS(TYPE_NTS type) {
	int index = static_cast<int>(type) - 0;
	if (index > 36) return "";
	if (index < 0) return "";
	return STRING_NTS_LIST[index];
}

void TerNode::print(std::ostream& os) {
	os << symbol << endl;
}

void NonTerNode::print(std::ostream& os) {
	for (auto it = children.begin(); it != children.end(); it++) {
		(*it)->print(os);
	}
	if (highlighted) os << name << endl;
}




