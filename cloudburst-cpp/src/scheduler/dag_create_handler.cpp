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

void dag_create_handler(string serialized, zmq::socket_t &dag_create_socket, SocketCache &pusher_cache, KvsClientInterface *kvs,
                        map <string, pair<Dag, set < string>>> &dags, BaseSchedulerPolicy &policy,
                        logger log,
                        map<string, unsigned> &call_frequency, unsigned num_replicas = 1){
    Dag dag;
    dag.ParseFromString(serialized);

    // We do not allow duplicate DAG names, so we return an error if the name is already taken
    if(dags.find(dag.name()) != dags.end()){
        GenericResponse error;
        error.set_success(false);
        error.set_error(DAG_ALREADY_EXISTS);
        string serialized;
        error.SerializeToString(&serialized);
//        kZmqUtil->send_string(serialized, &dag_create_socket);
        return;
    }

    log->info("Creating DAG {}.", (dag.name()));

    // We persist the DAG in the KVS, so other schedulers can read the DAG when they hear about it.
    LWWPairLattice<string> payload(TimestampValuePair<string>(generate_timestamp(0), serialized));
    kvs_put(kvs, dag.name(), serialize(payload), log, LatticeType::LWW);

    for(auto fname: dag.functions()){
        for (int i = 0; i < num_replicas; ++i) {
            // TODO: policy pin function
        }
    }

}