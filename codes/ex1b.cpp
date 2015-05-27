/*
 * Compile and run with:
 *
 * g++ -Wall -Wextra -std=c++11 ex1b.cpp && ./a.out
 *
 */

#include <iostream>
#include <map>

//#define DEBUG_MODE

class A {
private:
#ifdef DEBUG_MODE
#else
    std::map<int, int> m;
#endif
public:
    void add(int a, int b) {
        m[a] = b;
    }
    Ptr<std::map<int, int>> get_access() {
#ifdef DEBUG_MODE
#else
        return ptrFromRef(m);
#endif
    }
};

template<class T>
inline std::ostream& print_map(std::ostream& out, T& d)
{
    out << "{";
    for (auto p = d.begin(); p != d.end(); p++) {
        if (p != d.begin()) out << ", ";
        out << (p->first) << ": " << (p->second);
    }
    out << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::map<int, int> &d)
{
    return print_map(out, d);
}

int main() {
    Ptr<std::map<int, int>> ap;
    {
        A a;
        a.add(5, 6);
        a.add(6, 7);
        ap = a.get_access();
        std::cout << "a = " << *ap << std::endl; // OK
    }
    std::cout << "a = " << *ap << std::endl; // Dangling
    return 0;
}
