#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <chrono>
#include <unistd.h>
#include "poisson_random_number_generator.h"

using namespace std;

#define SLEEP_MULTIPLIER 7
#define NUM_STATIONS 4
#define MAX_INTELLIGENCE_STAFF 2

int N, M, x, y;
auto start_time = std::chrono::high_resolution_clock::now();
int completed_operations = 0;

int intelligence_staff_reader_count = 0;
bool is_logbook_writer_active = false;
vector<bool> is_station_occupied(NUM_STATIONS, false);

pthread_mutex_t station_locks[NUM_STATIONS];
pthread_mutex_t output_file_lock;
pthread_mutex_t intelligence_staff_lock;
pthread_mutex_t logbook_lock;
pthread_cond_t station_available[NUM_STATIONS];
pthread_cond_t write_logbook_available;
sem_t *unit_completion_sem;

class Operative
{
public:
    int id;
    int unit_id;
    int station_id;

    Operative(int id)
    {
        this->id = id;
        this->unit_id = (id - 1) / M;
        this->station_id = (((id - 1) % NUM_STATIONS) + 1) % NUM_STATIONS;
    }
};

vector<Operative> operatives;
vector<Operative> leaders;

void initialize()
{
    int num_units = N / M;
    unit_completion_sem = new sem_t[num_units];

    for (int i = 0; i < N; i++)
    {
        operatives.emplace_back(i + 1);
        if ((i + 1) % M == 0)
        {
            leaders.emplace_back(operatives[i]);
        }
    }

    pthread_mutex_init(&output_file_lock, NULL);
    pthread_mutex_init(&intelligence_staff_lock, NULL);
    pthread_mutex_init(&logbook_lock, NULL);
    pthread_cond_init(&write_logbook_available, NULL);

    for (int i = 0; i < NUM_STATIONS; i++)
    {
        pthread_mutex_init(&station_locks[i], NULL);
        pthread_cond_init(&station_available[i], NULL);
    }
    for (int i = 0; i < num_units; i++)
    {
        sem_init(&unit_completion_sem[i], 0, 0);
    }
}

void write_output(const string &output)
{
    pthread_mutex_lock(&output_file_lock);
    cout << output;
    pthread_mutex_unlock(&output_file_lock);
}

long long get_time()
{
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    return duration.count();
}

void *operative_activity(void *arg)
{
    Operative *op = (Operative *)arg;

    usleep(get_random_number() * SLEEP_MULTIPLIER);
    write_output("Operative " + to_string(op->id) + " has arrived at the station " + to_string(op->station_id + 1) + " at time " + to_string(get_time()) + "\n");

    pthread_mutex_lock(&station_locks[op->station_id]);
    while (is_station_occupied[op->station_id])
    {
        write_output("Operative " + to_string(op->id) + " is waiting for station " + to_string(op->station_id + 1) + " at time " + to_string(get_time()) + "\n");
        pthread_cond_wait(&station_available[op->station_id], &station_locks[op->station_id]);
    }
    is_station_occupied[op->station_id] = true;
    pthread_mutex_unlock(&station_locks[op->station_id]);

    usleep(x * SLEEP_MULTIPLIER);

    pthread_mutex_lock(&station_locks[op->station_id]);
    write_output("Operative " + to_string(op->id) + " has finished document recreation at time " + to_string(get_time()) + "\n");
    is_station_occupied[op->station_id] = false;
    pthread_cond_broadcast(&station_available[op->station_id]);
    pthread_mutex_unlock(&station_locks[op->station_id]);

    sem_post(&unit_completion_sem[op->unit_id]);

    return NULL;
}

void *leader_write(void *arg)
{
    Operative *leader = (Operative *)arg;

    for (int i = 0; i < M; i++)
    {
        sem_wait(&unit_completion_sem[leader->unit_id]);
    }
    write_output("Unit " + to_string(leader->unit_id + 1) + " has completed document recreation phase at time " + to_string(get_time()) + "\n");

    pthread_mutex_lock(&intelligence_staff_lock);
    while (intelligence_staff_reader_count > 0 || is_logbook_writer_active)
    {
        pthread_cond_wait(&write_logbook_available, &intelligence_staff_lock);
    }
    is_logbook_writer_active = true; // Mark that a writer is now active.
    pthread_mutex_unlock(&intelligence_staff_lock);

    pthread_mutex_lock(&logbook_lock);
    write_output("Unit " + to_string(leader->unit_id + 1) + " has started intelligence distribution at time " + to_string(get_time()) + "\n");
    usleep(y * SLEEP_MULTIPLIER);
    completed_operations++;
    write_output("Unit " + to_string(leader->unit_id + 1) + " has completed intelligence distribution at time " + to_string(get_time()) + "\n");
    pthread_mutex_unlock(&logbook_lock);

    pthread_mutex_lock(&intelligence_staff_lock);
    is_logbook_writer_active = false;
    pthread_cond_broadcast(&write_logbook_available);
    pthread_mutex_unlock(&intelligence_staff_lock);

    return NULL;
}

void *intelligence_staff_read(void *args)
{
    int staff_id = *((int *)args);

    while (true)
    {
        pthread_mutex_lock(&intelligence_staff_lock);
        while (is_logbook_writer_active)
        {
            pthread_cond_wait(&write_logbook_available, &intelligence_staff_lock);
        }
        intelligence_staff_reader_count++;
        if (intelligence_staff_reader_count == 1)
        {
            pthread_mutex_lock(&logbook_lock);
        }
        pthread_mutex_unlock(&intelligence_staff_lock);

        write_output("Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + to_string(get_time()) + ". " +
                     "Operations completed = " + to_string(completed_operations) + "\n");
        usleep(get_random_number());

        pthread_mutex_lock(&intelligence_staff_lock);
        intelligence_staff_reader_count--;
        if (intelligence_staff_reader_count == 0)
        {
            pthread_cond_signal(&write_logbook_available);
            pthread_mutex_unlock(&logbook_lock);
        }
        pthread_mutex_unlock(&intelligence_staff_lock);

        usleep(get_random_number() * SLEEP_MULTIPLIER);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./a.out <input_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile.is_open())
    {
        cout << "Error opening input file." << endl;
        return 1;
    }
    ofstream outputFile("output.txt");
    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outputFile.rdbuf());

    inputFile >> N >> M >> x >> y;
    inputFile.close();

    initialize();

    vector<pthread_t> op_threads(N);
    vector<pthread_t> leader_threads(leaders.size());
    vector<pthread_t> staff_threads(MAX_INTELLIGENCE_STAFF);
    vector<int> staff_ids(MAX_INTELLIGENCE_STAFF);

    for (int i = 0; i < N; i++)
        pthread_create(&op_threads[i], NULL, operative_activity, &operatives[i]);

    for (size_t i = 0; i < leaders.size(); i++)
        pthread_create(&leader_threads[i], NULL, leader_write, &leaders[i]);

    for (int i = 0; i < MAX_INTELLIGENCE_STAFF; i++)
    {
        staff_ids[i] = i + 1;
        pthread_create(&staff_threads[i], NULL, intelligence_staff_read, &staff_ids[i]);
    }

    for (size_t i = 0; i < leaders.size(); i++)
        pthread_join(leader_threads[i], NULL);

    for (int i = 0; i < N; i++)
        pthread_join(op_threads[i], NULL);

    usleep(get_random_number() * SLEEP_MULTIPLIER);
    for (int i = 0; i < MAX_INTELLIGENCE_STAFF; i++)
        pthread_cancel(staff_threads[i]);

    pthread_mutex_destroy(&output_file_lock);
    pthread_mutex_destroy(&intelligence_staff_lock);
    pthread_mutex_destroy(&logbook_lock);
    pthread_cond_destroy(&write_logbook_available);

    for (int i = 0; i < NUM_STATIONS; i++)
    {
        pthread_mutex_destroy(&station_locks[i]);
        pthread_cond_destroy(&station_available[i]);
    }
    for (int i = 0; i < N / M; i++)
    {
        sem_destroy(&unit_completion_sem[i]);
    }
    delete[] unit_completion_sem;

    std::cout.rdbuf(coutBuffer);
    outputFile.close();

    return 0;
}