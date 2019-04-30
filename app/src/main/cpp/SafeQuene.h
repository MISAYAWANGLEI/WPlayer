#include <pthread.h>
#include <queue>

using namespace std;

template <typename T>
class SafeQuene {
    typedef void (*releaseCallBack)(T&);//函数指针
private:
    queue<T> queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work;
    releaseCallBack mReleaseCallBack;

public:
    SafeQuene();
    ~SafeQuene();
    void setReleaseCallBack(releaseCallBack callBack);
    void setWork(int work);
    int empty();
    int size();
    void push(const T value);
    int pop(T& value);
    void clear();
};

