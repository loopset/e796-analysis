#ifndef MyHeader
#define MyHeader

namespace MyNamespace
{
    template <typename T>
    void func(T val);

    void over(int val){};
    void over(double val){};
    void over(float val){};

    void naive(){};

}

#endif // !MyHeader
