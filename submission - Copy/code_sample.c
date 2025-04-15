#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock;   // tạo mutex toàn cục
int shared_counter = 0; // biến dùng chung giữa các thread

void *increase_counter(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < 5; i++)
    {
        pthread_mutex_lock(&lock); // 🔒 lock mutex
        shared_counter++;
        printf("Thread %d tăng counter lên: %d\n", id, shared_counter);
        pthread_mutex_unlock(&lock); // 🔓 unlock mutex
        usleep(100000);              // nghỉ 100ms
    }

    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_mutex_init(&lock, NULL); // khởi tạo mutex

    pthread_create(&t1, NULL, increase_counter, &id1);
    pthread_create(&t2, NULL, increase_counter, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Giá trị cuối cùng của shared_counter: %d\n", shared_counter);

    pthread_mutex_destroy(&lock); // huỷ mutex

    return 0;
}
