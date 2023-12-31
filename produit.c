#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10
#define N 10

// Matrices
int B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE], A[MAX_SIZE][MAX_SIZE];

// Buffer
int T[N][MAX_SIZE];

// Synchronization variables
pthread_mutex_t mutex;
sem_t empty;
sem_t full;

// Function to generate a random matrix
void generateMatrix(int matrix[MAX_SIZE][MAX_SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % 10; // Fill with random values for illustration
        }
    }
}

// Function to perform matrix multiplication for a given row
void multiplyRow(int rowB, int colsB, int rowC, int colsC) {
    int sum = 0;
    for (int i = 0; i < colsB; i++) {
        sum += B[rowB][i] * C[i][rowC];
    }
    T[rowB][rowC] = sum;
}

// Producer function
void *producer(void *arg) {
    int row = *((int *)arg);
    for (int i = 0; i < MAX_SIZE; i++) {
        sem_wait(&empty); // Wait for an empty slot in the buffer
        pthread_mutex_lock(&mutex);

        // Calculate and insert the result into the buffer
        multiplyRow(row, MAX_SIZE, i, MAX_SIZE);

        pthread_mutex_unlock(&mutex);
        sem_post(&full); // Signal that a slot in the buffer is filled
    }
    pthread_exit(NULL);
}

// Consumer function
void *consumer(void *arg) {
    int col = *((int *)arg);
    for (int i = 0; i < N; i++) {
        sem_wait(&full); // Wait for a filled slot in the buffer
        pthread_mutex_lock(&mutex);

        // Retrieve the result from the buffer and place it in the matrix A
        A[i][col] = T[i][col];

        pthread_mutex_unlock(&mutex);
        sem_post(&empty); // Signal that a slot in the buffer is empty
    }
    pthread_exit(NULL);
}

int main() {
    // Initialize matrices and buffer with random values
    generateMatrix(B, MAX_SIZE, MAX_SIZE);
    generateMatrix(C, MAX_SIZE, MAX_SIZE);

    // Initialize synchronization variables
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);

    // Create producer threads
    pthread_t producers[MAX_SIZE];
    int producer_args[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; i++) {
        producer_args[i] = i;
        pthread_create(&producers[i], NULL, producer, (void *)&producer_args[i]);
    }

    // Create consumer threads
    pthread_t consumers[MAX_SIZE];
    int consumer_args[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; i++) {
        consumer_args[i] = i;
        pthread_create(&consumers[i], NULL, consumer, (void *)&consumer_args[i]);
    }

    // Join producer threads
    for (int i = 0; i < MAX_SIZE; i++) {
        pthread_join(producers[i], NULL);
    }

    // Join consumer threads
    for (int i = 0; i < MAX_SIZE; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Print the resulting matrix A
    printf("Resulting Matrix A:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }

    // Destroy synchronization variables
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
