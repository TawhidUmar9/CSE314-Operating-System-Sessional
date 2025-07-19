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

sem_t *station_locks;
sem_t output_file_lock;
sem_t intelligence_staff_lock;
sem_t logbook_lock;

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
    station_locks = new sem_t[NUM_STATIONS];

    for (int i = 0; i < N; i++)
    {
        operatives.emplace_back(i + 1);
        if ((i + 1) % M == 0)
        {
            leaders.emplace_back(operatives[i]);
        }
    }

    sem_init(&output_file_lock, 0, 1);
    sem_init(&intelligence_staff_lock, 0, 1);
    sem_init(&logbook_lock, 0, 1);
    for (int i = 0; i < NUM_STATIONS; i++)
        sem_init(&station_locks[i], 0, 1);

    for (int i = 0; i < num_units; i++)
        sem_init(&unit_completion_sem[i], 0, 0);
}

void write_output(const string &output)
{
    sem_wait(&output_file_lock);
    cout << output;
    sem_post(&output_file_lock);
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

    usleep(get_random_number() * SLEEP_MULTIPLIER); // Simulate time taken to arrive at the station
    write_output("Operative " + to_string(op->id) + " has arrived at the station " + to_string(op->station_id + 1) + " at time " + to_string(get_time()) + "\n");
    sem_wait(&station_locks[op->station_id]); // try to acquire the lock for the station
    usleep(x * SLEEP_MULTIPLIER);             // Simulate time taken to complete the operation
    write_output("Operative " + to_string(op->id) + " has finished document recreation at time " + to_string(get_time()) + "\n");
    sem_post(&station_locks[op->station_id]);
    sem_post(&unit_completion_sem[op->unit_id]); // Signal that the operative has completed their operation

    return NULL;
}

void *leader_write(void *arg)
{
    Operative *leader = (Operative *)arg;
    for (int i = 0; i < M; i++)
        sem_wait(&unit_completion_sem[leader->unit_id]);
    write_output("Unit " + to_string(leader->unit_id + 1) + " has completed document recreation phase\n");
    sem_wait(&logbook_lock);
    write_output("Unit " + to_string(leader->unit_id + 1) + " has started intelligence distribution\n");
    usleep(y * SLEEP_MULTIPLIER);
    completed_operations++;
    write_output("Unit " + to_string(leader->unit_id + 1) + " has completed intelligence distribution\n");
    sem_post(&logbook_lock);

    return NULL;
}

void *intelligence_staff_read(void *args)
{
    int staff_id = *((int *)args);
    while (true)
    {
        sem_wait(&intelligence_staff_lock);
        intelligence_staff_reader_count++;
        if (intelligence_staff_reader_count == 1)
            sem_wait(&logbook_lock);
        sem_post(&intelligence_staff_lock);
        write_output("Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + to_string(get_time()) + ". " +
                     "Operations completed = " + to_string(completed_operations) + "\n");
        double random_number = get_random_number() / 300.0;
        usleep(random_number * SLEEP_MULTIPLIER);
        sem_wait(&intelligence_staff_lock);
        intelligence_staff_reader_count--;
        if (intelligence_staff_reader_count == 0)
            sem_post(&logbook_lock);
        sem_post(&intelligence_staff_lock);
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

    sem_destroy(&output_file_lock);
    sem_destroy(&intelligence_staff_lock);
    sem_destroy(&logbook_lock);

    for (int i = 0; i < N / M; i++)
        sem_destroy(&unit_completion_sem[i]);
    for (int i = 0; i < NUM_STATIONS; i++)
        sem_destroy(&station_locks[i]);
    delete[] station_locks;
    delete[] unit_completion_sem;

    std::cout.rdbuf(coutBuffer);
    outputFile.close();

    return 0;
}