//  Copyright 2019 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.


#ifndef PROJECT_SCHEDULER_POLICY_INTERFACE_HPP
#define PROJECT_SCHEDULER_POLICY_INTERFACE_HPP

#include "server_utils.hpp"
#include "types.hpp"
#include "cloudburst.pb.h"
#include "internal.pb.h"

using ThreadLocation = pair<Address, unsigned>;

class SchedulerPolicyInterface {
public:
    // This scheduler's IP address.
    string ip_;

    zmq::socket_t* pin_accept_socket_ptr;

    SocketCache* pusher_cache_ptr;

    KvsClientInterface* kvs_client_;

    pmap<ThreadLocation, set<unsigned>> running_counts_;

    pmap<ThreadLocation, TimePoint> backoff_;

    map<string, set<Address>> key_locations_;

    hset<ThreadLocation, pair_hash> unpinned_executors_;

    map<string, hset<ThreadLocation, pair_hash>> function_locations_;

    map<string, vector<pair<string, ThreadLocation>>> pending_dags_;

    pmap<ThreadLocation, ThreadStatus> thread_statuses_;

    float random_threshold_;

    hset<ThreadLocation, pair_hash> unique_executors_;

    bool local_;

    logger log_;

public:
    SchedulerPolicyInterface(zmq::socket_t &pin_accept_socket, SocketCache &pusher_cache, KvsClientInterface* kvs,
            string ip, logger log, float random_threshold=0.20, bool local=false):
    ip_(ip),
    pin_accept_socket_ptr(&pin_accept_socket),
    pusher_cache_ptr(&pusher_cache),
    kvs_client_(kvs),
    running_counts_(pmap<ThreadLocation, set<unsigned>>()),
    backoff_(pmap<ThreadLocation, TimePoint>()),
    key_locations_(map<string, set<Address>>()),
    unpinned_executors_(hset<ThreadLocation, pair_hash>()),
    function_locations_(map<string, hset<ThreadLocation, pair_hash>>()),
    pending_dags_(map<string, vector<pair<string, ThreadLocation>>>()),
    thread_statuses_(pmap<ThreadLocation, ThreadStatus>()),
    random_threshold_(random_threshold),
    unique_executors_(),
    local_(local),
    log_(log) {}

    virtual ThreadLocation pick_executor(const vector<string>& references, string function_name = "") = 0;

    virtual bool pin_function(string dag_name, string function_name) = 0;

    virtual void commit_dag(string dag_name) = 0;

    virtual void discard_dag(const Dag& dag, bool pending=false) = 0;

    virtual void process_status(const ThreadStatus& status) = 0;

    virtual void update() = 0;

    virtual void update_function_locations(SchedulerStatus& status) = 0;
};

extern SchedulerPolicyInterface* kSchedulerPolicy;

#endif //PROJECT_SCHEDULER_POLICY_INTERFACE_HPP
