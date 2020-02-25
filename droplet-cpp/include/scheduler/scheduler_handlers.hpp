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
#include "client/kvs_client.hpp"

void connect_handler(zmq::socket_t &connect_socket, string route_addr, logger log);

void function_create_handler(zmq::socket_t &func_create_socket, KvsClient *kvs, ConsistencyType consistency = NORMAL,
                             logger log);

void function_call_handler(zmq::socket_t &func_call_socket,
                           SocketCache &pusher_cache, BaseDropletSchedulerPolicy &policy, logger log);

void dag_create_handler(zmq::socket_t &dag_create_socket, SocketCache &pusher_cache, KvsClient *kvs,
                        map <string, pair<Dag, set < string>> &dags, BaseDropletSchedulerPolicy &policy,
                        map<string, unsigned> &call_frequency, unsigned num_replicas = 1, logger log);

void dag_call_handler(zmq::socket_t &dag_call_socket, SocketCache &pusher_cache, map<string, unsigned> &last_arrivals,
                      map<string, unsigned> &interarrivals, map <string, pair<Dag, set < string>> &dags,
                      BaseDropletSchedulerPolicy &policy, map<string, unsigned> &call_frequency, logger log);

void dag_delete_handler(zmq::socket_t &dag_delete_socket, map <string, pair<Dag, set < string>> &dags,
                        BaseDropletSchedulerPolicy &policy, map<string, unsigned> &call_frequency, logger log);

void list_handler(zmq::socket_t &list_socket, KvsClient *kvs, logger log);

void exec_status_handler(zmq::socket_t &exec_status_socket, BaseDropletSchedulerPolicy &policy, logger log);

void sched_update_handler(zmq::socket_t &sched_update_socket, KvsClient *kvs, BaseDropletSchedulerPolicy &policy,
                          map<string, unsigned> &call_frequency, logger log);

#endif //DROPLET_SCHEDULER_HANDLERS_HPP
