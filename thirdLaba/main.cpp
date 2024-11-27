#include <iostream>
#include <vector>
#include <pthread.h>
#include <chrono>
#include <limits>
#include <numeric>
#include <algorithm>
#include <mutex>
#include <csignal>
#include <cstdlib> 

using namespace std;

struct MarkerData {
    int                threadId;
    int                arraySize;
    int                signalToMain;
    int                shouldContinue;
    int*               array;
    pthread_mutex_t*   sharedMutex;//statoc?
    pthread_mutex_t*   personalMutex;
    pthread_cond_t*    condForBegin;//static?
    pthread_cond_t*    personalCondition;
    bool isActive;

    MarkerData(pthread_mutex_t* sharedMutex, int threadId, int* array, int arraySize, pthread_cond_t* condForBegin){
        this->threadId = threadId;
        this->array = array;
        this->arraySize = arraySize;
        this->sharedMutex = sharedMutex;
        this->condForBegin = condForBegin;

        signalToMain = 0;
        shouldContinue = 1;
        isActive = 1;

        personalMutex = new pthread_mutex_t;      
        pthread_mutexattr_t *mutexattr = new pthread_mutexattr_t;    
        pthread_mutexattr_init(mutexattr);
        pthread_mutex_init(personalMutex, mutexattr);

        personalCondition = new pthread_cond_t;
        pthread_condattr_t *condattr = new pthread_condattr_t;
        pthread_condattr_init(condattr);
        pthread_cond_init(personalCondition, condattr);

        pthread_mutexattr_destroy(mutexattr);
        pthread_condattr_destroy(condattr);
    }
    MarkerData(){                    
        personalMutex = new pthread_mutex_t;                                    
        pthread_mutexattr_t mutexattr;    
        pthread_mutexattr_init(&mutexattr);
        pthread_mutex_init(personalMutex, &mutexattr);
        pthread_mutexattr_destroy(&mutexattr);
    };

    MarkerData operator = (MarkerData* obj) {
        threadId = obj->threadId;
        array = obj->array;
        arraySize = obj->arraySize;
        sharedMutex = obj->sharedMutex;
        condForBegin = obj->condForBegin;
        return *this;
    }

    void unmarkArr(){

    }
};


void* markerThread(void* a){
    // cout << a << endl;
    MarkerData threadData = *(MarkerData*)a;
    pthread_mutex_lock(threadData.personalMutex);
    cout << threadData.threadId << "-ый маркер захватил мьютекс" << endl;
    threadData.signalToMain = 1;
    pthread_cond_wait(threadData.condForBegin, threadData.personalMutex);
    pthread_mutex_unlock(threadData.personalMutex);
    

    srand(threadData.threadId+1);

    pthread_mutex_lock(threadData.sharedMutex);
    while (threadData.shouldContinue)
    {
        int randomValue  = rand();
        int numberOfMarkedElements(0);
        randomValue %= threadData.arraySize+1;
        pthread_mutex_lock(threadData.sharedMutex);
        if(threadData.array[randomValue] == 0){
            sleep(1);
            threadData.array[randomValue] = threadData.threadId+1;
            numberOfMarkedElements++;
            sleep(1);
        }
        else
        {
            cout << "Поток номер " << threadData.threadId+1 << "покрасил" << numberOfMarkedElements << "элементов" 
                 << "приостановив работу на " << randomValue << "-ом элементе" << endl;
            threadData.signalToMain = 1;
            pthread_cond_wait(threadData.personalCondition, threadData.sharedMutex);
        }
        
        pthread_mutex_unlock(threadData.sharedMutex);
    }

    pthread_mutex_lock(threadData.sharedMutex);
    for(int i = 0; i < threadData.arraySize; i++){
        if(threadData.array[i] == threadData.threadId+1)
            threadData.array[i] = 0;
    }
    pthread_mutex_unlock(threadData.sharedMutex);

    return 0;
};


int main() {

    int error;
    int n;
    cout << "Enter the number of elements in the array: ";
    cin >> n;
    int* arr = new int[n];

    for(int i = 0; i < n; i++){
        arr[i] = 0;
    }

    int numberOfThreads;
    cout << "Enter the numer of threads: ";
    cin >> numberOfThreads;

    pthread_cond_t    condforbegin  = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t   mtx           = PTHREAD_MUTEX_INITIALIZER;

    MarkerData* markerData =  new MarkerData[numberOfThreads];
    pthread_t* pthreads =  new pthread_t[numberOfThreads];
    for(int i = 0; i < numberOfThreads; i++){
        markerData[i] = new MarkerData(&mtx, i , arr, n, &condforbegin);
        if(pthread_create(pthreads + i, NULL, markerThread, markerData + i)){
            cout << "ошибка при создании потока" << endl;
        }
    }


    while(true){
        int sum  = 0;
        for(int i = 0; i < numberOfThreads; i++){
            pthread_mutex_lock(markerData[i].personalMutex);
            sum += markerData[i].signalToMain;
            pthread_mutex_unlock(markerData[i].personalMutex);
        }
        if(sum == numberOfThreads)
            break;
        sleep(1);
    }


    for(int i = 0; i < numberOfThreads; i++){
            markerData[i].signalToMain = 0;
    }


    pthread_cond_broadcast(&condforbegin);//bigining of the actual work of threads

    int NumberOfActiveThreads = numberOfThreads;
    while(NumberOfActiveThreads){
        while(true){
            int sum  = 0;
            for(int i = 0; i < numberOfThreads; i++){
                pthread_mutex_lock(markerData[i].personalMutex);
                sum += markerData[i].signalToMain;
                pthread_mutex_unlock(markerData[i].personalMutex);
            }
            if(sum == numberOfThreads)
                break;
            sleep(1);
        }

        cout << "массив: ";
        for(int i = 0; i < n; i++){
            cout << arr[i] << " ";
        }

        int currentThread;
        cout << "с каким маркером работаем?"  << endl;
        cin >> currentThread; 
        currentThread--;
        if(markerData[currentThread].isActive == false){
            cout << "поток неактивен, попробуйте ещё раз" << endl;
            continue;
        }
        markerData[currentThread].isActive = false;
        pthread_cond_signal(markerData[currentThread].personalCondition);   

        NumberOfActiveThreads--;

    }



    void* d;
    for(int i = 0; i < numberOfThreads; i++){
        pthread_join(*(pthreads+i), &d);
    }
    pthread_mutex_destroy(&mtx);

    return 0;
}


// int pthread_cond_signal(pthread_cond_t *cond);
// int pthread_cond_broadcast(pthread_cond_t *cond);
// int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);