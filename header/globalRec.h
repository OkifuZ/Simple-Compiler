#ifndef GLOBALREC
#define GLOBALREC

#include <string>
#include <vector>

class GlobalStringList {
private:
    
public:
    std::vector<std::string> list;
    void addStringConst(std::string str) {
        list.push_back(str);
    }



};










#endif // GLOBALREC