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
#include "scheduler/scheduler_handlers.hpp"

void dag_call_handler(string serialized,
                      zmq::socket_t &dag_call_socket,
                      SocketCache &pusher_cache,
                      map<string, unsigned> &last_arrivals,
                      map<string, unsigned> &interarrivals,
                      map<string, pair<Dag, set<string>>> &dags,
                      BaseSchedulerPolicy &policy,
                      map<string, unsigned> &call_frequency,
                      logger log){

}