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

void dag_call_handler(string serialized,
                      zmq::socket_t &dag_call_socket,
                      SocketCache &pusher_cache,
                      map<string, TimePoint> &last_arrivals,
                      map<string, vector<unsigned long long>> &interarrivals,
                      map<string, pair<Dag, set<string>>> &dags,
                      BaseSchedulerPolicy &policy,
                      map<string, unsigned> &call_frequency,
                      logger log){
    DagCall call;
    call.ParseFromString(serialized);
    string name = call.name();

    // check if dag exists
    if(dags.find(name) == dags.end()){
        GenericResponse response;
        response.set_success(false);
        response.set_error(CloudburstError::NO_SUCH_DAG);
        string serialized_error;
        kZmqUtil->send_string(serialized_error, &dag_call_socket);
        return;
    }

    auto t = std::chrono::system_clock::now();

    if(last_arrivals.find(name) != last_arrivals.end()){
        if(interarrivals.find(name) == interarrivals.end()){
            interarrivals.insert(pair<string, vector<unsigned long long>>(name, vector<unsigned long long>()));
        }
        interarrivals.at(name).push_back(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                        t - last_arrivals.at(name)
                        ).count());
    }

    auto it = last_arrivals.find(name);
    if(it != last_arrivals.end()){
        it->second = t;
    }
    else{
        last_arrivals.insert(pair<string, TimePoint>(name, t));
    } //TODO: shorter version?

    // retrieve Dag object from cache
    pair<Dag, set<string>> dag_sources_pair = dags.at(name);
    Dag dag = dag_sources_pair.first;
    set<string> sources = dag_sources_pair.second;
    for(auto fname : dag.functions()){
        call_frequency.at(fname) += 1;
    }

    double start_time = std::chrono::duration<double>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    // actuall call the dag
    DagSchedule schedule;
    schedule.set_id(get_random_response_key());
    schedule.set_allocated_dag(&dag); // TODO: CopyFrom? Why no set_dag?
    schedule.set_start_time(start_time);
    schedule.set_consistency(call.consistency());
    
    if(call.response_address() != ""){
        schedule.set_response_address(call.response_address());
    }
    if(call.output_key() != ""){
        schedule.set_output_key(call.output_key());
    }
    if(call.client_id() != ""){
        schedule.set_client_id(call.client_id());
    }

    // construct DagSchedule object
    for(auto fname : dag.functions()){
        vector<string> refs;
        for(auto ref: call.references()){
            refs.push_back(ref);
        }
        // try to assign executors for each function
        pair<Address, unsigned> result = policy.pick_executor(refs);
        if (result.first == ""){
            GenericResponse response;
            response.set_success(false);
            response.set_error(CloudburstError::NO_RESOURCES);
            string serialized_response;
            response.SerializeToString(&serialized_response);
            kZmqUtil->send_string(serialized_response, &dag_call_socket);
            return;
        }

        // record assigned locations
        Address ip = result.first;
        unsigned tid = result.second;
        auto locations_ptr = schedule.mutable_locations();
        (*locations_ptr)[fname] = ip + ':' + std::to_string(tid);

        // copy over arguments
        auto func_args_ptr = call.mutable_function_args();
        auto sched_args_ptr = schedule.mutable_arguments();
        (*sched_args_ptr)[fname].values().MergeFrom(*func_args_ptr)[fname].values());
        //TODO: copying over ^
    }

    // send DagSchedule's for all functions
    for(auto fname : dag.functions()){
        auto locations_ptr = schedule.mutable_locations();
        string location = (*locations_ptr)[fname];
        std::size_t ind = location.find(":");
        string ip = get_queue_address(location.substr(0, ind), std::stoul(location.substr(ind+1, location.size())));
        schedule.set_target_function(fname);

        // get predecessors/parents of function
        vector<string> triggers = get_dag_predecessors(dag, fname);
        // if no predecessors, it's a source
        if(triggers.empty()){
            triggers.push_back("BEGIN");
        }

        // clear triggers from last function and add triggers for this one
        schedule.clear_triggers();
        for(string trigger : triggers){
            schedule.triggers().Add(trigger);
        }
        string serialized_schedule;
        schedule.SerializeToString(&serialized_schedule);
        kZmqUtil->send_string(serialized_schedule, &pusher_cache[ip]);
    }

    // Send triggers for sources
    for(string source : sources){
        DagTrigger trigger;
        trigger.set_id(schedule.id());
        trigger.set_source("BEGIN");
        trigger.set_target_function(source);

        string ip = get_dag_trigger_address(schedule.locations().at(source));
        string serialized_trigger;
        trigger.SerializeToString(&serialized_trigger);
        kZmqUtil->send_string(serialized_trigger, &pusher_cache[ip]);
    }

    // Send ok response to client
    GenericResponse response;
    response.set_success(true);
    if(!schedule.output_key().empty()){
    response.set_response_id(schedule.output_key());
    } else {
    response.set_response_id(schedule.id());
    }
    string serialized_response;
    response.SerializeToString(&serialized_response);
    kZmqUtil->send_string(serialized_response, &dag_call_socket);
    return;
}