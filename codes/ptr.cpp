// g++ -std=c++11 -O3 -march=native a.cpp -S && c++filt < a.s > a.sf

#include <type_traits>

enum ENull { null };

template<class T>
class Ptr {
public:
    // Constructors from other types
    inline Ptr(ENull null_in=null) : ptr_(0) {}
    inline explicit Ptr(T *ptr) : ptr_(ptr) { }
    template<class T2> inline Ptr(const Ptr<T2>& ptr) : ptr_(ptr.get()) {}

    // We want Ptr to be TriviallyCopyable, so we must use the default copy
    // constructor and assingment, default move constructor and assignment and
    // default destructor.
    ~Ptr() = default;
    inline Ptr(const Ptr<T>&) = default;
    Ptr<T>& operator=(const Ptr<T>&) = default;
    inline Ptr(Ptr&&) = default;
    Ptr<T>& operator=(Ptr&&) = default;

    inline T* operator->() const { return ptr_; }
    inline T& operator*() const { return *ptr_; }
    inline T* get() const { return ptr_; }
    inline T* getRawPtr() const { return get(); }
    inline const Ptr<T> ptr() const { return *this; }
private:
    T *ptr_;
};

struct Foo {
  int i, j;
  Foo(int i_, int j_) : i(i_), j(j_) {}
};

static_assert(std::is_trivially_copyable<Ptr<Foo>>::value, "Ptr<Foo> is not trivially copyable");
static_assert(std::is_standard_layout<Ptr<Foo>>::value, "Ptr<Foo> is not standard layout");

// False -- Ptr<T> is trivially copyable, but not trivially default
// constructible
//static_assert(std::is_trivial<Ptr<Foo>>::value, "Ptr<Foo> is not trivial");


Ptr<Foo> getf1(Ptr<Foo> p);
Foo*     getf2(Foo*     p);

int f1()
{
  Ptr<Foo> p;
  p = getf1(p);
  int r = p->i + p->j;
  delete p.get();
  return r;
}

int f2()
{
  Foo *p;
  p = getf2(p);
  int r = p->i + p->j;
  delete p;
  return r;
}
