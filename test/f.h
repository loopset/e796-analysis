#ifndef MyHeader
#define MyHeader

inline void outer() {};

namespace MyNamespace
{
    template <typename T>
    void func(T val);

    void over(int val){};
    void over(double val){};
    void over(float val){};

    void inner(){};

}

#endif // !MyHeader
