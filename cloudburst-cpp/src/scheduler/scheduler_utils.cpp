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

#include "scheduler/scheduler_utils.hpp"

//VectorClock DEFAULT_VC = VectorClock();
//DEFAULT_VC.insert("base", MaxLattice<unsigned>(1));

set<string> get_ip_set(string request_ip, SocketCache socket_cache){
    kZmqUtil->send_string("", &socket_cache[request_ip]);

    StringSet ips;
    ips.ParseFromString(kZmqUtil->recv_string(&socket_cache[request_ip]));
    set<string> result;

    for(string ip : ips.keys()){
        result.insert(ip);
    }
    return result;
}
// Note: there is an additional bool argument `exec_threads` in the Python version,
// but this function is never called with exec_threads=True, so only the case with
// exec_threads=False is implemented

set<string> find_dag_source(Dag dag){
    set<string> sinks;
    for(const auto& conn : dag.connections()){
        sinks.insert(conn.sink());
    }

    set<string> funcs;
    for(const auto& func : dag.functions()){
        if(sinks.find(func) != sinks.end()){
            funcs.insert(func);
        }
    }

    return funcs;
}

VectorClock get_default_vc(){
    VectorClock DEFAULT_VC = VectorClock();
    DEFAULT_VC.insert("base", MaxLattice<unsigned>(1));
    return DEFAULT_VC;
}