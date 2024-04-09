/**
 *  scheduler cpp file
 * 
 *  @author: Matteo Cusanelli
 *  @date: December 2023
*/


#include "scheduler.h"
#include <vector>
#include <string>
#include <algorithm>
#include<numeric>
#include <limits>


/**
 * Updates the prev_value and and the seq output
*/
int update_seq(int cpu, int next_value, int & prev_value, int64_t max_seq_len, std::vector<int> & seq){
    // Adds to the seq vector if the following if conditions are true
    if (cpu == next_value){
        if (cpu != prev_value){ // If the previous value and the new one are the same, doesnt add to seq
            if((int64_t)seq.size() < max_seq_len) {
                seq.push_back(cpu);     
                prev_value = cpu; // updates the previous value put into seq
            }
            else{
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Gets the process with the smallest burst time
*/
int64_t smallest_burst_time(std::vector<int64_t> all_bursts){
    auto smallest_burst_index = std::min_element(all_bursts.begin(), all_bursts.end());
    while (*smallest_burst_index == 0){
        all_bursts.erase(smallest_burst_index);
        smallest_burst_index = std::min_element(all_bursts.begin(), all_bursts.end());
    }
    return *smallest_burst_index;
}

/**
 *  Checks the job queue and adds any processes that have arrived
*/
void check_job_queue(std::vector<int> & Job_queue, std::vector<int> & Ready_queue, std::vector<Process> & processes, int64_t & curr_time){
    for (auto & p : Job_queue){
        auto &new_process = processes.at(Job_queue.front());
        if (curr_time > new_process.arrival_time){
            Ready_queue.push_back(new_process.id);
            Job_queue.erase(Job_queue.begin());
        }
    }
}


// this is the function you should implement
//
// simulate_rr() implements a Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the compressed execution sequence
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//         - sequence will be trimmed to contain only first max_seq_len entries
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//
void simulate_rr(
    int64_t quantum, 
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) 
{
    /**
     * VECTOR DESCRIPTIONS
     * Job queue: Vector containing all the processes not in the ready queue with the first process being at the front
     * Ready queue: Vector containing all the processes in the ready queue
     * has_started: Vector containing 0 or 1 in each cell for each process (with id being the index). 0 - process has not started, 1 - process has started 
     * remaining_burstss: Vector containing all the remaining bursts of the processes, with each cell corresponding to a process id
     * ready_bursts: Vector containing all of the remaining bursts of the processes in the ready queue
     * 
     * OTHER DESCRIPTIONS
     * next_value: id of process that is going to be executed or moved into the ready queue
     * cpu: id of process currently being executed
     * prev_value: Previous value of the last process that was added to the seq vector
     * process: process from the processes vector
     * 
     * FOR bonmus:
     * n: Number of times to update the quantum for each process in the ready queue
     * arrival_time: Arrival time of a process (for the bonus)
     * new_time: current time + quantum * Ready_queue.size
     * all_started: Integer displaing if all processes in ready queue have started with 1 - all started and 0 - not all have started
    */
    std::vector<int> Job_queue, Ready_queue, has_started;
    std::vector<int64_t> remaining_bursts, ready_bursts;
    int64_t curr_time, N, arrival_time, new_time;
    int next_value, cpu, prev_value, all_started;
    seq.clear();

    // Adding all the process to the their respective queues and update the other vectors
     for (auto & p : processes){
        Job_queue.push_back(p.id);
        remaining_bursts.push_back(p.burst);
        has_started.push_back(0);
     }

    // Initializing staring values
    curr_time = 0;
    prev_value = -2;

    while(1){
        all_started = 1;
        cpu = -1;

        // -------------------------------- Case where both queues are empty -------------------
        if (Job_queue.empty() && Ready_queue.empty()){ 
            break;
        }

        // ------------------------------- Case where Ready queue is empty ------------------------
        else if (Ready_queue.empty()){ 
            next_value = Job_queue.front();
            auto &process = processes.at(next_value);
            curr_time = process.arrival_time;

            // Update job and ready queue
            Ready_queue.push_back(process.id);
            Job_queue.erase(Job_queue.begin());

            if (curr_time != 0){ // If a process does not immediately get put into ready queue at time 0
                next_value = -1;
            }
        }

        // ------------------------------- Case where job queue is empty -------------------------
        else if (Job_queue.empty()){
            next_value = Ready_queue.front();
            auto &process = processes.at(next_value);
            cpu = process.id;

            // Checks to see if the process has started or not, if it has not, change its start time and set it in has_started to 1
            if (has_started.at(process.id) == 0){
                process.start_time = curr_time;
                has_started.at(process.id) = 1;
            }

            if (quantum >= remaining_bursts.at(next_value)){ // If the remaining time of process < quantum
                curr_time += remaining_bursts.at(next_value);
                process.finish_time = curr_time;

                // Update the remaining burst time of the process and remove from ready queue
                remaining_bursts.at(next_value) = 0;
                Ready_queue.erase(Ready_queue.begin());
            }
            else { // If the remaining time of process > quantum
                curr_time += quantum;
                remaining_bursts.at(process.id) -= quantum;

                // Add process back to the end of the ready queue 
                Ready_queue.erase(Ready_queue.begin());
                Ready_queue.push_back(process.id);
            }
        }

        // -------------------------- If both queues are not empty ------------------------------
        else{   
            next_value = Ready_queue.front();
            auto &process = processes.at(next_value);
            cpu = process.id;

            // Checks to see if the process has started or not, if it has not, change its start time and set it in has_started to 1
            if (has_started.at(process.id) == 0){
                process.start_time = curr_time;
                has_started.at(process.id) = 1;
            }
            if (quantum >= remaining_bursts.at(next_value)){ // If the remaining time of process < quantum
                curr_time += remaining_bursts.at(next_value);
                process.finish_time = curr_time;

                check_job_queue(Job_queue, Ready_queue, processes, curr_time);

                Ready_queue.erase(Ready_queue.begin());
                remaining_bursts.at(next_value) = 0;
            }
            else { // If the remaining time of process > quantum
                curr_time += quantum;
                remaining_bursts.at(process.id) -= quantum;

                check_job_queue(Job_queue, Ready_queue, processes, curr_time);
            
                // Add process back to the end of the ready queue
                Ready_queue.erase(Ready_queue.begin());
                Ready_queue.push_back(process.id);
            }
        }

        update_seq(cpu, next_value, prev_value, max_seq_len, seq);

        // Set the all started variable to 0 if any process in the ready queue has not started yet
        for (auto i: Ready_queue){
            if (has_started.at(i) == 0){
                all_started = 0;
            }
        }

        for (auto p : Ready_queue){ // Get the remaining_bursts of each process in the ready queue and save it to the ready_bursts vector
            ready_bursts.push_back(remaining_bursts.at(p));
        }


        // ------------------------------- IF the ready queue contains processes (for bonus) -----------------------
        if (!Ready_queue.empty() && curr_time != 0 && all_started == 1){ 

            if (Job_queue.empty()){ // If the job queue is empty, increase arrival time to the max value so we can pass the if conditions below
                arrival_time = std::numeric_limits<int64_t>::max();
                new_time = arrival_time;
            }else{ // If the job queue is not empty, set the arrival time to the first process in the queue
                auto next_to_arrive = processes.at(Job_queue.front()); 
                arrival_time = next_to_arrive.arrival_time;
                new_time = curr_time + (quantum * (int64_t)Ready_queue.size()); // time taking into account the arrival time + quantum for each process in  ready queue
            }


            // If the next arrival time from the job queue and smallest burst time in the ready queue are > quantum
            if (smallest_burst_time(ready_bursts) > quantum && arrival_time >= new_time){ 
                // If the smallest burst time is smaller than the arrival time of the next in job queue
                if (arrival_time > smallest_burst_time(ready_bursts)){ 
                    N = smallest_burst_time(ready_bursts);
                }
                // If the first process in the job queue arrives before the smallest burst time of a process in the ready queue
                else{
                    N = arrival_time - curr_time;
                }

                // Update N below
                N = N / quantum;
                if (N % quantum == 0){
                    N --;
                }
                N = N / (int64_t)Ready_queue.size();

                // If current time after updating is greater than the arrival time of the next item in the job queue
                if ((curr_time + (int64_t)Ready_queue.size() * quantum * N) > arrival_time){ 
                    N = arrival_time - curr_time;
                    N = N / quantum;
                    if (N % quantum == 0){
                        N --;
                    }
                    N = N / (int64_t)Ready_queue.size();
                }

                curr_time += ((int64_t)Ready_queue.size() * quantum * N); // Update the current time

                // Update the burst times of each process in the ready queue
                for (auto &process : Ready_queue){
                    auto &new_burst_time = remaining_bursts.at(process);
                    new_burst_time -= N * quantum;
                }
                
                int64_t N_copy = N;
                while(N_copy > 0){ // Update the seq vector
                    if (Ready_queue.size() == 1){ // If there is only 1 process in the ready queue
                        cpu = Ready_queue.front();
                        update_seq(cpu, Ready_queue.front(), prev_value, max_seq_len, seq);
                        N_copy = 0; // set N_copy = 0 to break from the loop

                    }else{ // If there are multiple processes in the ready queue
                        for (auto New_process : Ready_queue){
                            cpu = New_process;
                            if (update_seq(cpu, New_process, prev_value, max_seq_len, seq) == 1){ // If the seq vector has reached capacity
                                N_copy = 0; // Set N_copy = 0 to break from the loop
                            }
                        }
                    }
                    N_copy--; 
                }
            }
        }
        ready_bursts.clear();
    }
}
