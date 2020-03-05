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

#ifndef DROPLET_SERVER_UTILS_HPP
#define DROPLET_SERVER_UTILS_HPP

#include "lattices/core_lattices.hpp"
#include "client/kvs_client.hpp"
#include "yaml-cpp/yaml.h"
#include "droplet.pb.h"
#include "common.hpp"

using VectorClock = MapLattice <string, MaxLattice<unsigned>>;

const string FUNC_PREFIX = "funcs/";
const string BIND_ADDR_TEMPLATE = "tcp://*:%d";

const unsigned PIN_PORT = 4000;
const unsigned UNPIN_PORT = 4010;
const unsigned FUNC_EXEC_PORT = 4020;
const unsigned DAG_QUEUE_PORT = 4030;
const unsigned DAG_EXEC_PORT = 4040;
const unsigned SELF_DEPART_PORT = 4050;

const unsigned STATUS_PORT = 5007;
const unsigned SCHED_UPDATE_PORT = 5008;
const unsigned BACKOFF_PORT = 5009;
const unsigned PIN_ACCEPT_PORT = 5010;

const unsigned RECV_INBOX_PORT = 5500;

const unsigned STATISTICS_REPORT_PORT = 7006;

//// Create a generic error response protobuf.
//GenericResponse resp;
//resp.set_success(false);
//
//// Create a generic success response protobuf.
//GenericResponse ok = GenericResponse();
//ok.set_success(true);
//
//string serialized_ok;
//ok.SerializeToString(&serialized_ok);
//
//// Create a default vector clock for keys that have no dependencies.
//VectorClock DEFAULT_VC = VectorClock();
//DEFAULT_VC.insert("base", MaxLattice<unsigned>(1));

inline string get_func_kvs_name(string fname) {
    return FUNC_PREFIX + fname;
}

//inline string get_dag_trigger_address(string address) {
//    vector <string> tokens;
//    split(address, ":", tokens); TODO: split
//    string ip, tid;
//    ip = tokens[0];
//    tid = tokens[1];
//    return "tcp://" + ip + ":" + std::to_string(std::stoi(tid) + DAG_EXEC_PORT);
//}

inline string get_statistics_report_address(string mgmt_ip) {
    return "tcp://" + mgmt_ip + ":" + std::to_string(STATISTICS_REPORT_PORT);
}

inline string get_backoff_addresss(string ip) {
    return "tcp://" + ip + ":" + std::to_string(BACKOFF_PORT);
}

inline string get_pin_accept_port(string ip) {
    return "tcp://" + ip + ":" + std::to_string(PIN_ACCEPT_PORT);
}

vector <string> get_dag_predecessors(Dag dag, string fname);

inline string get_user_msg_inbox_addr(string ip, string tid) {
    return "tcp://" + ip + ":" + std::to_string(std::stoi(tid) + RECV_INBOX_PORT);
}


YAML::Node load_conf(string filename){
    return YAML::LoadFile(filename);
}

bool kvs_put(KvsClientInterface *kvs, string key, string value, logger log);

string kvs_get(KvsClientInterface *kvs, string key, logger log);

// generate_timestamp implemented in common.hpp

#endif //DROPLET_SERVER_UTILS_HPP
