#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>
// mutex lock
pthread_mutex_t queue_lock;

int empty(struct queue_t *q)
{
    if (q == NULL)
        return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    pthread_mutex_lock(&queue_lock); // Khóa mutex trước khi truy cập hàng đợi
    /* Thêm tiến trình mới vào hàng đợi [q] nếu chưa đầy */
    if (q->size < MAX_QUEUE_SIZE)
    {
        q->proc[q->size++] = proc; // Gán tiến trình vào vị trí cuối hàng và tăng size
    }
    pthread_mutex_unlock(&queue_lock); // Mở khóa mutex sau khi truy cập hàng đợi
}

// Hàm lấy tiến trình có độ ưu tiên cao nhất ra khỏi hàng đợi
struct pcb_t *dequeue(struct queue_t *q)
{
    pthread_mutex_lock(&queue_lock); // Khóa mutex trước khi truy cập hàng đợi
    /* Trả về tiến trình có độ ưu tiên cao nhất trong hàng đợi [q]
     * và loại bỏ tiến trình đó khỏi hàng
     */
    if (empty(q))
    {
        return NULL; // Nếu hàng đợi rỗng, trả về NULL
    }

    // **Hiện tại:** hàm giả định rằng phần tử đầu tiên là có độ ưu tiên cao nhất
    struct pcb_t *proc = q->proc[0]; // Lưu tiến trình đầu tiên

    // Dời các phần tử còn lại sang trái để lấp chỗ trống
    for (int i = 1; i < q->size; i++)
    {
        q->proc[i - 1] = q->proc[i];
    }

    q->size--; // Giảm kích thước hàng đợi sau khi loại bỏ phần tử

    pthread_mutex_unlock(&queue_lock); // Mở khóa mutex sau khi truy cập hàng đợi
    return proc;                       // Trả về tiến trình đã lấy ra
}
