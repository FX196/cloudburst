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

#ifndef DROPLET_SCHEDULER_UTILS_HPP
#define DROPLET_SCHEDULER_UTILS_HPP

// utils.py

const unsigned NUM_EXEC_THREADS = 3;

const unsigned EXECUTORS_PORT = 7002;
const unsigned SCHEDULERS_PORT = 7004;

//vector<string> get_func_list(KvsClient client, string prefix, bool fullname=false);
//
//void put_func_list(KvsClient client, vector<string> funclist);

inline string get_cache_ip_key(string ip){
    return "ANNA_METADATA|cache_ip|" + ip;
}

inline string get_pin_address(string ip, unsigned tid){
    return "tcp://" + ip + ":" + std::to_string(PIN_PORT + tid);
}

inline string get_unpin_address(string ip, unsigned tid){
    return "tcp://" + ip + ":" + std::to_string(UNPIN_PORT + tid);
}

inline string get_exec_address(string ip, unsigned tid){
    return "tcp://" + ip + ":" + std::to_string(FUNC_EXEC_PORT + tid);
}

inline string get_queue_address(string ip, unsigned tid){
    return "tcp://" + ip + ":" + std::to_string(DAG_QUEUE_PORT + tid);
}

inline string get_scheduler_list_address(string mgmt_ip){
    return "tcp://" + mgmt_ip + ":" + std::to_string(SCHEDULERS_PORT);
}

inline string get_scheduler_update_address(string ip){
    return "tcp://" + ip + ":" + std::to_string(SCHED_UPDATE_PORT);
}

set<string> get_ip_set(string request_ip, SocketCache &socket_cache);
// Note: there is an additional bool argument `exec_threads` in the Python version,
// but this function is never called with exec_threads=True, so only the case with
// exec_threads=False is implemented

set<string> find_dag_source(Dag dag);

VectorClock get_default_vc();

// call.py

// moved to scheduler_handlers.hpp: call_function, call_dag

//GenericResponse call_dag(DagCall call, SocketCache pusher_cache, map<string, pair<Dag, set<string>>> dags, BaseSchedulerPolicy &policy);


// create.py

// moved to scheduler_handlers.hpp: create_function, create_dag, delete_dag



#endif //DROPLET_SCHEDULER_UTILS_HPP
