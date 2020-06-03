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
                        map <string, pair<Dag, set<string>>> &dags, SchedulerPolicyInterface *policy,
                        map<string, unsigned> &call_frequency, logger log, unsigned num_replicas){
    Dag dag;
    dag.ParseFromString(serialized);


    // We do not allow duplicate DAG names, so we return an error if the name is already taken
    if(dags.find(dag.name()) != dags.end()){
        GenericResponse error;
        error.set_success(false);
        error.set_error(CloudburstError::DAG_ALREADY_EXISTS);
        string serialized;
        error.SerializeToString(&serialized);
        kZmqUtil->send_string(serialized, &dag_create_socket);
        return;
    }

    std::cout << "Creating dag, name: " << dag.name() << std::endl;

    log->info("Creating DAG {}.", (dag.name()));

    // try to pin all functions in the dag
    for(auto& func_reference: dag.functions()){
        for (int i = 0; i < num_replicas; ++i) {
            // policy will return false if there are no executors to pin this function
            std::cout << "pinning function " << func_reference.name() << std::endl;
            if(!(policy->pin_function(dag.name(), func_reference))){
                log->info("Creating DAG %s failed due to insufficient resources", dag.name());
                GenericResponse error;
                error.set_success(false);
                error.set_error(CloudburstError::NO_RESOURCES);
                string serialized;
                error.SerializeToString(&serialized);
                kZmqUtil->send_string(serialized, &dag_create_socket);

                // unpin all previously pinned functions
                policy->discard_dag(dag, true);
                return;
            }
        }
    }

    std::cout << "Dag functions pinned" << std::endl;

    // We persist the DAG in the KVS, so other schedulers can read the DAG when they hear about it.
    LWWPairLattice<string> payload(TimestampValuePair<string>(generate_timestamp(0), serialized));
    kvs_put(kvs, dag.name(), serialize(payload), log, LatticeType::LWW);

    for(auto& func_reference: dag.functions()){
        string fname = func_reference.name();
        if(call_frequency.find(fname) == call_frequency.end()){
            call_frequency.insert(pair<string, unsigned>(fname, 0));
        }
    }

    // Only create this metadata after all functions have been successfully created
    policy->commit_dag(dag.name());
    pair<Dag, set<string>> dag_sources_pair(dag, find_dag_source(dag));
    dags.insert(pair<string, pair<Dag, set<string>>>(dag.name(), dag_sources_pair));

    std::cout << "Committing dag" << std::endl;

    // Send ok response
    GenericResponse response;
    response.set_success(true);
    string serialized_response;
    response.SerializeToString(&serialized_response);
    kZmqUtil->send_string(serialized_response, &dag_create_socket);
}
