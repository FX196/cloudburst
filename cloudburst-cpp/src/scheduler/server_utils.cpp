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

vector <string> get_dag_predecessors(Dag dag, string fname) {
    vector <string> result;
    for (const Link &connection : dag.connections) {
        if (fname.compare(connection.sink) == 0) {
            result.push_back(connection.source);
        }
    }
    return result;
}

bool kvs_put(KvsClientInterface *kvs, string key, string value, logger log){
    string rid = kvs->put_async(key, value, LatticeType::LWW);
    vector<KeyResponse> responses = kvs->receive_async();
    while (responses.size() == 0) {
        responses = kvs->receive_async();
    }
    KeyResponse response = responses[0];
    if (response.tuples()[0].error() == AnnaError::NO_ERROR){
        log->error("PUT request errored! Key: %s", key);
        return false;
    }
    return true;
}

string kvs_get(KvsClientInterface *kvs, string key, logger log){
    kvs->get_async(key);

    vector<KeyResponse> responses = client->receive_async();
    while (responses.size() == 0) {
        responses = client->receive_async();
    }

    if (responses.size() > 1) {
        log->error("Error: received more than one response");
    }
    assert(responses[0].tuples(0).lattice_type() == LatticeType::LWW);
    LWWPairLattice<string> lww_lattice =
            deserialize_lww(responses[0].tuples(0).payload());
    return lww_lattice.reveal().value;
}

