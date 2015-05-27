/*
 * Compile and run with:
 *
 * g++ -Wall -Wextra -std=c++11 ex1.cpp && ./a.out
 *
 */

#include <iostream>
#include <map>

class A {
private:
    std::map<int, int> m;
public:
    void add(int a, int b) {
        m[a] = b;
    }
    std::map<int, int> *get_access() {
        return &m;
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
    std::map<int, int> *ap;
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
