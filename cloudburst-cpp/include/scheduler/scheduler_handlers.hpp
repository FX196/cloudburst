//  Copyright 2019 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License", logger log);
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

#ifndef DROPLET_SCHEDULER_HANDLERS_HPP
#define DROPLET_SCHEDULER_HANDLERS_HPP

#include "scheduler_utils.hpp"
#include "server_utils.hpp"
#include "policy/scheduler_policy_interface.hpp"

void connect_handler(string serialized, zmq::socket_t &connect_socket, string route_addr, logger log);

void function_create_handler(string serialized, zmq::socket_t &func_create_socket, KvsClientInterface *kvs, logger log, ConsistencyType consistency = NORMAL);

void function_call_handler(string serialized, zmq::socket_t &func_call_socket,
                           SocketCache &pusher_cache, SchedulerPolicyInterface *policy, logger log);

void dag_create_handler(string serialized,
        zmq::socket_t &dag_create_socket,
        SocketCache &pusher_cache,
        KvsClientInterface *kvs,
        map<string, pair<Dag, set<string>>> &dags,
        SchedulerPolicyInterface *policy,
        map<string, unsigned> &call_frequency,
        logger log,
        unsigned num_replicas=1);

void dag_call_handler(string serialized,
        zmq::socket_t &dag_call_socket,
        SocketCache &pusher_cache,
        map<string, TimePoint> &last_arrivals,
        map<string, vector<unsigned long long>> &interarrivals,
        map<string, pair<Dag, set <string>>> &dags,
        SchedulerPolicyInterface *policy,
        map<string, unsigned> &call_frequency,
        logger log,
        string request_id="");

void dag_delete_handler(string dag_name, zmq::socket_t &dag_delete_socket, map <string, pair<Dag, set < string>>> &dags,
                        SchedulerPolicyInterface *policy, map<string, unsigned> &call_frequency, logger log);

void list_handler(string serialized, zmq::socket_t &list_socket, KvsClientInterface *kvs, logger log);

void exec_status_handler(string serialized, zmq::socket_t &exec_status_socket, SchedulerPolicyInterface *policy, logger log);

void sched_update_handler(string serialized, zmq::socket_t &sched_update_socket, KvsClientInterface *kvs, SchedulerPolicyInterface *policy,
                          map<string, unsigned> &call_frequency, logger log);

#endif //DROPLET_SCHEDULER_HANDLERS_HPP
