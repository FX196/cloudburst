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
#include "server_utils.hpp"
#include "anna.pb.h"

vector <string> get_dag_predecessors(Dag dag, string fname) {
    vector <string> result;
    for (const Dag::Link &connection : dag.connections()) {
        if (fname.compare(connection.sink()) == 0) {
            result.push_back(connection.source());
        }
    }
    return result;
}

//bool kvs_put(KvsClientInterface *kvs, string key, string value, logger log){
//    string rid = kvs->put_async(key, value, LatticeType::LWW);
//    vector<KeyResponse> responses = kvs->receive_async();
//    while (responses.size() == 0) {
//        responses = kvs->receive_async();
//    }
//    KeyResponse response = responses[0];
//    if (response.tuples()[0].error() == AnnaError::NO_ERROR){
//        log->error("PUT request errored! Key: {}", key);
//        return false;
//    }
//    return true;
//}

bool kvs_put(KvsClientInterface *kvs, string key, string value, logger log, LatticeType type) {
    string rid = kvs->put_async(key, value,
                                   type); // TODO: check put SingleKeyCausalLattice<SetLattice<string>>

    vector <KeyResponse> responses = kvs->receive_async();
    while (responses.size() == 0) {
        responses = kvs->receive_async();
    }

    KeyResponse response = responses[0];

    if (response.response_id() != rid) {
        log->error("Invalid response: ID did not match request ID!");
    }
    if (response.tuples()[0].error() == AnnaError::NO_ERROR) {
        log->info("Successfully put key-value pair.");
        return true;
    } else {
        log->error("Failure on put of key-value pair!");
        return false;
    }

}

//Lattice kvs_get(KvsClientInterface *kvs, string key, logger log, LatticeType type) {
//    kvs->get_async(key);
//
//    vector <KeyResponse> responses = kvs->receive_async();
//    while (responses.size() == 0) {
//        responses = kvs->receive_async();
//    }
//
//    if (responses.size() > 1) {
//        log->error("Error: received more than one response");
//    }
//    assert(responses[0].tuples(0).lattice_type() == type);
//    switch (type) {
//        case LatticeType::LWW:
//            LWWPairLattice <string> lww_lattice =
//                    deserialize_lww(responses[0].tuples(0).payload());
//            return lww_lattice;
//        case LatticeType::SET:
//            SetLattice <string> set_lattice =
//                    deserialize_set(responses[0].tuples(0).payload());
//            return set_lattice;
////        case LatticeType::SINGLE_CAUSAL:
////            SingleKeyCausalLattice<SetLattice<string>> single_causal_lattice = // always this template?
////                    // TODO: not that I think we need this, but I want to know how it works just in case
//    }
//}

string kvs_get(KvsClientInterface *kvs, string key, logger log, LatticeType type) {
    kvs->get_async(key);

    vector <KeyResponse> responses = kvs->receive_async();
    while (responses.size() == 0) {
        responses = kvs->receive_async();
    }

    if (responses.size() > 1) {
        log->error("Error: received more than one response");
    }
    assert(responses[0].tuples(0).lattice_type() == type);
    return responses[0].tuples(0).payload();
}

vector <string> get_func_list(KvsClientInterface *kvs, string prefix, logger log, bool fullname) {
    string serialized = kvs_get(kvs, FUNCOBJ, log, LatticeType::SET);
    SetLattice <string> funcobj = deserialize_set(serialized);
    set <string> funcs = funcobj.reveal();
    prefix = FUNC_PREFIX + prefix;
    vector <string> result;
    for (const string &func : funcs) {
        // TODO: do we need to decode utf8?
        if (func.find(prefix) == 0) { // TODO: startswith
            if (!fullname) {
                result.push_back(func.substr(prefix.length()));
            } else {
                result.push_back(func);
            }
        }
    }
    return result;
}

void put_func_list(KvsClientInterface *kvs, vector <string> funclist, logger log) {
    set<string> funcset(funclist.begin(), funclist.end());
    SetLattice <string> lattice(funcset);
    kvs_put(kvs, FUNCOBJ, serialize(lattice), log, LatticeType::SET);
}

string get_random_id(size_t length){
    auto randchar = []() -> char
    {
        const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}
string get_dag_trigger_address(string address){
    std::size_t ind = address.find(":");
    string ip = address.substr(0, ind);
    string tid = address.substr(ind+1, address.size());
    return "tcp://" + ip + ":" + std::to_string(std::stoi(tid) + DAG_EXEC_PORT);
}