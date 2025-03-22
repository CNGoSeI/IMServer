#ifndef SINGLETONTEMPLATE_H
#define SINGLETONTEMPLATE_H

template <typename T>
class TSingleton
{
private:
    TSingleton() = default;
    TSingleton(const TSingleton<T>&) = delete;
    TSingleton& operator=(const TSingleton<T>&) = delete;
    friend T;//为了正常访问 使其成为友元
public:
    static T& GetInstance() {
        static T Ins;
        return Ins;
    }

    virtual ~TSingleton() = default;  // 直接默认实现
};

#endif // !1
