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

#ifndef PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
#define PROJECT_DEFAULT_SCHEDULER_POLICY_HPP

#include "scheduler_policy_interface.hpp"

class DefaultSchedulerPolicy : public SchedulerPolicyInterface {
public:
    // This scheduler's IP address.
//    string ip_;
//
//    zmq::socket_t* pin_accept_socket_ptr;
//
//    SocketCache* pusher_cache_ptr;
//
//    KvsClientInterface* kvs_client;
//
//    pmap<ThreadLocation, set<unsigned>> running_counts;
//
//    pmap<ThreadLocation, TimePoint> backoff;
//
//    map<string, set<Address>> key_locations;
//
//    hset<ThreadLocation, pair_hash> unpinned_executors;
//
//    map<string, hset<ThreadLocation, pair_hash>> function_locations;
//
//    map<string, vector<pair<string, ThreadLocation>>> pending_dags;
//
//    pmap<ThreadLocation, ThreadStatus> thread_statuses;
//
//    float random_threshold;
//
//    bool local;
//
//    logger log;

public:
//    DefaultSchedulerPolicy(zmq::socket_t &pin_accept_socket, SocketCache &pusher_cache, KvsClientInterface* kvs,
//            string ip, logger log, float random_threshold=0.20, bool local=false):
//            ip_(ip),
//            pin_accept_socket_ptr(&pin_accept_socket),
//            pusher_cache_ptr(&pusher_cache),
//            kvs_client(kvs),
//            running_counts(pmap<ThreadLocation, set<unsigned>>()),
//            backoff(pmap<ThreadLocation, TimePoint>()),
//            key_locations(map<string, set<Address>>()),
//            unpinned_executors(),
//            function_locations(map<string, hset<ThreadLocation, pair_hash>>()),
//            pending_dags(map<string, vector<pair<string, ThreadLocation>>>()),
//            thread_statuses(pmap<ThreadLocation, ThreadStatus>()),
//            random_threshold(random_threshold),
//            unique_executors_(hset<ThreadLocation, pair_hash>()),
//            local(local),
//            log(log) {}

    DefaultSchedulerPolicy(zmq::socket_t &pin_accept_socket, SocketCache &pusher_cache, KvsClientInterface* kvs,
                           string ip, logger log, float random_threshold=0.20, bool local=false) :
            SchedulerPolicyInterface(pin_accept_socket, pusher_cache,  kvs,
                                     ip, log, random_threshold, local) {}

    ThreadLocation pick_executor(const vector<string>& references, string function_name){
        // Construct a map which maps from IP addresses to the number of
        // relevant arguments they have cached. For the time begin, we will
        // just pick the machine that has the most number of keys cached.
        map<string, unsigned> arg_map;
        hset<ThreadLocation, pair_hash> executors;

        if(!function_name.empty()){
            executors = hset<ThreadLocation, pair_hash>(function_locations.at(function_name));
        } else {
            executors = hset<ThreadLocation, pair_hash>(unpinned_executors);
        }
        for(auto executor_time_point_pair : backoff){
            executors.erase(executor_time_point_pair.first);
        }
        for(auto executor_counts_pair : running_counts){
            if((executor_counts_pair.second.size() > 1000) && (get_random_double() > random_threshold)){
                executors.erase(executor_counts_pair.first);
            }
        }

        if(executors.size() == 0){
            return ThreadLocation("", 0);
        }

        set<Address> executor_ips;
        for(ThreadLocation executor : executors){
            executor_ips.insert(executor.first);
        }

        std::cout << "generated candidates" << std::endl;

        // Count number of references cached at each executor
        for(string reference : references){
            if(key_locations.find(reference) != key_locations.end()){
                set<Address> ips = key_locations.at(reference);
                for(Address ip : ips){
                    if(executor_ips.find(ip) != executor_ips.end()){
                        if(arg_map.find(ip) == arg_map.end()){
                            arg_map.insert({ip, 0});
                        }
                        arg_map.at(ip)  += 1;
                    }
                }
            }
        }

        // Get IP with max amount of keys cached
        Address max_ip;
        unsigned max_count = 0;
        for(auto ip_count_pair : arg_map){
            if(ip_count_pair.second > max_count){
                max_ip = ip_count_pair.first;
            }
        }

        ThreadLocation max_executor;
        if(!max_ip.empty()){
            vector<ThreadLocation> candidates;
            for(auto executor : executors){
                if(executor.first.compare(max_ip) == 0){
                    candidates.push_back(executor);
                }
            }
            max_executor = candidates[rand() % candidates.size()];
        }

        std::cout << "Found max_executor " << std::endl;

        if(max_ip.empty() || (get_random_double() < random_threshold)){
            auto r = rand() % executors.size();
            auto it = executors.begin();
            std::advance(it, r);
            max_executor = *it;
        }

        std::cout << "Chosen executor " << std::endl;

//        if(!running_counts.contains(max_executor)){
//            running_counts.insert(make_pair(max_executor, set<unsigned>()));
//        }
        running_counts[max_executor].insert(get_time_since_epoch());

        if(function_name.empty()){
            unpinned_executors.erase(max_executor);
        }

        unique_executors.insert(max_executor);
        return max_executor;
    }

    bool pin_function(string dag_name, const Dag::FunctionReference& func_ref){
        if(unpinned_executors.size() == 0){
            return false;
        }

        if(pending_dags.find(dag_name) == pending_dags.end()){
            pending_dags.insert(make_pair(dag_name, vector<pair<string, ThreadLocation>>()));
        }

        hset<ThreadLocation, pair_hash> candidates(unpinned_executors);

        while(true){
            auto r = rand() % candidates.size();
            auto it = candidates.begin();
            std::advance(it, r);
            ThreadLocation executor = *it;

            kZmqUtil->send_string(ip + ":" + func_ref.name(),
                    &(*pusher_cache_ptr)[get_pin_address(executor.first, executor.second)]);

            GenericResponse response;
            try {
                response.ParseFromString(kZmqUtil->recv_string(&(*pin_accept_socket_ptr)));
            } catch (const zmq::error_t&){
                log->error("Pin operation to %s:%u timed out. Retrying.",
                        executor.first, executor.second);
                continue;
            }

            unpinned_executors.erase(executor);
            candidates.erase(executor);

            if(response.success()){
                pending_dags.at(dag_name).push_back({func_ref.name(), executor});
                return true;
            }
            else {
                log->error("Node %s:%u rejected pin for %s. Retrying.",
                        executor.first, executor.second, func_ref.name());
                continue;
            }
        }
    }

    void commit_dag(string dag_name){
        for(auto name_location_pair : pending_dags.at(dag_name)){
            string function_name = name_location_pair.first;
            ThreadLocation location = name_location_pair.second;
            if(function_locations.find(function_name) == function_locations.end()){
                function_locations[function_name] = hset<ThreadLocation, pair_hash>();
            }
            function_locations[function_name].insert(location);
        }
        pending_dags.erase(dag_name);
    }

    void discard_dag(const Dag& dag, bool pending=false){
        vector<pair<string, ThreadLocation>> pinned_locations;
        if(pending){
            pinned_locations = pending_dags.at(dag.name());
        }
        else {
            for(auto function_ref : dag.functions()){
                for(auto location : function_locations.at(function_ref.name())){
                    pinned_locations.push_back(pair<string, ThreadLocation>(function_ref.name(), location));
                }
            }
        }

        for(auto name_location_pair : pinned_locations){
            string fname = name_location_pair.first;
            ThreadLocation location = name_location_pair.second;
            kZmqUtil->send_string(
                    fname,
                    &(*pusher_cache_ptr)[get_unpin_address(location.first, location.second)]
                    );
        }
    }

    void process_status(const ThreadStatus& status){
        ThreadLocation key = ThreadLocation(status.ip(), status.tid());
        log->info("Received status update from executor %s:%u", key.first, key.second);

        if(!status.running()){
            // this means that this node is departing, so we remove it from all
            // of our metatdata tracking.
            if(thread_statuses.find(key) != thread_statuses.end()){
                for(auto fname : thread_statuses.at(key).functions()){
                    function_locations.at(fname).erase(key);
                    thread_statuses.erase(key);
                }
            }
            unpinned_executors.erase(key);
            return;
        }

        if(status.functions_size() == 0){
            unpinned_executors.insert(key);
        }

        if((thread_statuses.find(key) != thread_statuses.end())&&
            !(MessageDifferencer::Equivalent(thread_statuses.at(key), status))){
            // remove all old function locations, add all new ones
            for(auto fname : thread_statuses.at(key).functions()){
                if(function_locations.find(fname) != function_locations.end()){
                    function_locations.erase(fname);
                }
            }
        }

        thread_statuses.insert(pair<ThreadLocation, ThreadStatus>(key, status));

        for(string fname : status.functions()){
            function_locations[fname].insert(key);
        }
        if(status.utilization() > 0.70 && !local){
            bool not_lone_executor = true;
            for(string fname : status.functions()){
                not_lone_executor = not_lone_executor && (function_locations.at(fname).size() > 1);
            }
            if(not_lone_executor){
                backoff[key] = std::chrono::system_clock::now();
            }
        }
    }

    void update(){
        // periodically clean up the running counts map to drop any times older
        // than 5 seconds
        for(auto executor_counts_pair : running_counts){
            set<unsigned> saved_counts;
            unsigned now = get_time_since_epoch();
            for(unsigned ts : executor_counts_pair.second){
                if(now - ts < 5000){
                    saved_counts.insert(ts);
                }
            }
            running_counts[executor_counts_pair.first] = saved_counts;
        }

        // clean up any backoff messages that were added more than 5 seconds ago
        hset<ThreadLocation, pair_hash> remove_set;
        TimePoint now = std::chrono::system_clock::now();
        for(auto executor_time_pair : backoff){
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                    now - executor_time_pair.second)
                    .count();
            if(duration > 5){
                remove_set.insert(executor_time_pair.first);
            }
        }

        for(ThreadLocation executor : remove_set){
            backoff.erase(executor);
        }

        set<Address> executor_ips;
        for(auto location_status_pair : thread_statuses){
            executor_ips.insert(location_status_pair.second.ip());
        }

        key_locations.clear();
        for(Address ip : executor_ips){
            string key = get_cache_ip_key(ip);
            string serialized_lattice = kvs_get(kvs_client, key, log, LatticeType::LWW);
            if(serialized_lattice.empty()){
                continue;
            }
            LWWPairLattice<string> lattice = deserialize_lww(serialized_lattice);
            StringSet st;
            st.ParseFromString(lattice.reveal().value);

            for(auto key : st.keys()){
                key_locations[key].insert(ip);
            }
        }
    }

    void update_function_locations(SchedulerStatus& status){
        for(auto location : status.function_locations()){
            string function_name = location.name();
            function_locations[function_name].insert(
                    ThreadLocation(location.ip(), location.tid()));
        }
    };
};



#endif //PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
