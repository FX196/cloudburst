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

void run_scheduler(string ip, string mgmt_ip, string rout_addr){

    ZmqUtil zmq_util;
    ZmqUtilInterface *kZmqUtil = &zmq_util;

    // set up logging
    string log_file = "log_" + std::to_string(ip) + ".txt";
    string log_name = "scheduler_log_" + std::to_string(ip);
    auto log = spdlog::basic_logger_mt(log_name, log_file, true);
    log->flush_on(spdlog::level::info);

    bool local = mgmt_ip.compare("") == 0;
    KvsClient kvs(); //TODO: fill in constructor

    // A mapping from a DAG's name to its protobuf representation.
    map<string, pair<Dag, set<string>> dags;

    // Tracks how often a request for each function is received.
    map<string, unsigned> call_frequency;

    // Tracks the most recent arrival for each DAG -- used to calculate interarrival times.
    map<string, unsigned> last_arrivals;

    // Tracks the time interval between successive requests for a particular DAG.
    map<string, unsigned> interarrivals;

    // Maintains a list of all other schedulers in the system, so we can propagate metadata to them.
    vector<string> schedulers;

    zmq::context_t context(1);

}