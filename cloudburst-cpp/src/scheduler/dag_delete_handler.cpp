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

void dag_delete_handler(string dag_name, zmq::socket_t &dag_delete_socket, map <string, pair<Dag, set<string>>> &dags,
SchedulerPolicyInterface *policy, map<string, unsigned> &call_frequency, logger log){
    if(dags.find(dag_name) == dags.end()){
        GenericResponse error;
        error.set_success(false);
        error.set_error(CloudburstError::NO_SUCH_DAG);
        string serialized_error;
        error.SerializeToString(&serialized_error);
        kZmqUtil->send_string(serialized_error, &dag_delete_socket);
        return;
    }
    Dag dag = dags.at(dag_name).first;
    policy->discard_dag(dag);

    for(string fname : dag.functions()){
        call_frequency.erase(fname);
    }
    dags.erase(dag_name);
    GenericResponse ok;
    ok.set_success(true);
    string serialized_ok;
    ok.SerializeToString(&serialized_ok);
    kZmqUtil->send_string(serialized_ok, &dag_delete_socket);
    log->info("Dag %s deleted", dag_name);
}