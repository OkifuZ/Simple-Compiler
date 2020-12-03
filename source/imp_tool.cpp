#include <string>
#include <iostream>
#include <sstream>

#include "../header/tool.h"

using namespace std;

string upperCase_tool(const string& s) {
    string ans;
    char c;
    for (auto it = s.begin(); it != s.end(); it++) {
        c = *it;
        if (c <= 'z' && c >= 'a') c += 'A' - 'a';
        ans += c;
    }
    return ans;
}

bool idenSame(string a, string b) {
    return (upperCase_tool(a) == upperCase_tool(b));
}

int str2int(std::string s) {
    int ans = 0;
    char c;
    int i;
    bool neg = false;
    if (s[0]=='-') {
        neg = true;
        i = 1;
    }
    else if (s[0] == '+') {
        i = 1;
    }
    else {
        i = 0;
    }
    for (; i < s.size(); i++) {
        c = s[i];
        if (c > '9' || c < '0') {
            printPos(88754);
            cout << "sefw" << endl;
        }
        else ans = ans * 10 + (c - '0');
    }
    return neg ? -ans : ans;
}

string int2str(int n) {
    ostringstream os;
    os << n;
    return os.str();
}

char str2char(std::string s) {
    if (s.size() > 1) printPos(8069);
    return s[0];
}

void printPos(int i){
#ifdef PRINT_ERROR_MESSAGE
    cout << "error!!!" << " " << i << endl;
#endif // PRINT_ERROR_MESSAGE
}

bool isIntstr(string s) {
    int i;
    if (s[0] == '-' || s[0] == '+') {
        i = 1;
    }
    else {
        i = 0;
    }
    for (; i < s.size(); i++) {
        if (s[i] > '9' || s[i] < '0') {
            if (i == 0 && (s[i] == '+' || s[i] == '-')) {
                // pass
            }
            else return false;
        }
    }
    return true;
}

