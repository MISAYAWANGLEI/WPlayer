#include <pthread.h>
#include <queue>

using namespace std;

template <typename T>
class SafeQuene {
    typedef void (*releaseCallBack)(T*);//函数指针
private:
    queue<T> queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work;
    releaseCallBack mReleaseCallBack;

public:
    SafeQuene(){
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&cond,NULL);
    }

    ~SafeQuene(){
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    void setReleaseCallBack(releaseCallBack callBack) {
        mReleaseCallBack = callBack;
    }

    void setWork(int work){
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int empty(){
        return queue.empty();
    }

    int size(){
        return queue.size();
    }

    void push(const T value){
        pthread_mutex_lock(&mutex);
        if(work){
            queue.push(value);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    int pop(T& value){
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while(work && queue.empty()){
            pthread_cond_wait(&cond,&mutex);
        }
        if (!queue.empty()){
            value = queue.front();
            queue.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void clear(){
        pthread_mutex_lock(&mutex);
        int size = queue.size();
        for (int i = 0; i < size; ++i) {
            T value = queue.front();
            mReleaseCallBack(value);
            queue.pop();
        }
        pthread_mutex_unlock(&mutex);
    }
};

