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
    string ip;

    zmq::socket_t* pin_accept_socket_ptr;

    SocketCache* pusher_cache_ptr;

    KvsClientInterface* kvs_client;

    pmap<ThreadLocation, set<unsigned>> running_counts;

    pmap<ThreadLocation, TimePoint> backoff;

    map<string, set<Address>> key_locations;

    hset<ThreadLocation, pair_hash> unpinned_executors;

    map<string, hset<ThreadLocation, pair_hash>> function_locations;

    map<string, vector<pair<string, ThreadLocation>>> pending_dags;

    pmap<ThreadLocation, ThreadStatus> thread_statuses;

    float random_threshold;

    hset<ThreadLocation, pair_hash> unique_executors;

    bool local;

    logger log;

public:
    SchedulerPolicyInterface() {}

    SchedulerPolicyInterface(zmq::socket_t &pin_accept_socket, SocketCache &pusher_cache, KvsClientInterface* kvs,
            string ip, logger log, float random_threshold=0.20, bool local=false):
    ip(ip),
    pin_accept_socket_ptr(&pin_accept_socket),
    pusher_cache_ptr(&pusher_cache),
    kvs_client(kvs),
    running_counts(),
    backoff(),
    key_locations(),
    unpinned_executors(),
    function_locations(),
    pending_dags(),
    thread_statuses(),
    random_threshold(random_threshold),
    unique_executors(),
    local(local),
    log(log) {}

    virtual ThreadLocation pick_executor(const vector<string>& references, string function_name = "") = 0;

    virtual bool pin_function(string dag_name, const Dag::FunctionReference& func_ref) = 0;

    virtual void commit_dag(string dag_name) = 0;

    virtual void discard_dag(const Dag& dag, bool pending=false) = 0;

    virtual void process_status(const ThreadStatus& status) = 0;

    virtual void update() = 0;

    virtual void update_function_locations(SchedulerStatus& status) = 0;
};

extern SchedulerPolicyInterface* kSchedulerPolicy;

#endif //PROJECT_SCHEDULER_POLICY_INTERFACE_HPP
