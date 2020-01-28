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

#include "common.hpp"
#include "client/kvs_client.hpp"

const unsigned METADATA_THRESHOLD = 5;
const unsigned REPORT_THRESHOLD = 5;

void run_scheduler(string ip, string mgmt_ip, vector<UserRoutingThread> routing_threads){
    bool local = mgmt_ip.compare("") == 0;
    KvsClient kvs(); //TODO: fill in constructor

    // A mapping from a DAG's name to its protobuf representation.
    map<string, pair<Dag, set<string>> dags;

    // Tracks how often a request for each function is received.
    map<string, unsigned> call_frequency;

    zmq::context_t context(1);

}