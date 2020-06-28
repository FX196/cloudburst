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

#ifndef PROJECT_TEST_LIST_FUNCS_HPP
#define PROJECT_TEST_LIST_FUNCS_HPP

#include "scheduler/policy/default_scheduler_policy.hpp"

TEST_F(PolicyTest, IgnoreOverloaded){
    string ip = "127.0.0.1";
    kSchedulerPolicy->unpinned_cpu_executors.insert({ip, 1});
    kSchedulerPolicy->unpinned_cpu_executors.insert({ip, 2});

    kSchedulerPolicy->backoff[{ip, 1}] = std::chrono::system_clock::now();
    for(int i=0; i<1100; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        kSchedulerPolicy->running_counts[{ip, 2}].insert(get_time_since_epoch());
    }

    ThreadLocation result = kSchedulerPolicy->pick_executor({}, "");
    EXPECT_EQ(result.first.empty(), true);
    EXPECT_EQ(result.second, 0);
}

TEST_F(PolicyTest, PinReject){
    string ip = "127.0.0.1";
    kSchedulerPolicy->unpinned_cpu_executors.insert({ip, 1});
    kSchedulerPolicy->unpinned_cpu_executors.insert({ip, 2});

    GenericResponse ok;
    ok.set_success(true);
    GenericResponse error;
    error.set_success(false);
    string ok_msg, error_msg;
    ok.SerializeToString(&ok_msg);
    error.SerializeToString(&error_msg);
    add_zmq_response(error_msg);
    add_zmq_response(ok_msg);

    Dag::FunctionReference fref;
    fref.set_name("function");

    bool success = kSchedulerPolicy->pin_function("dag", fref, {});
    EXPECT_EQ(success, true);

    // Ensure that both remaining executors have been removed from unpinned
    // and that the DAG commit is pending.
    EXPECT_EQ(kSchedulerPolicy->unpinned_cpu_executors.size(), 0);
    EXPECT_EQ(kSchedulerPolicy->pending_dags.size(), 1);
}

TEST_F(PolicyTest, ProcessStatus){
    string function_name = "square";
    ThreadStatus status;
    status.set_running(true);
    status.set_ip("127.0.0.1");
    status.set_tid(1);
    string* func_ptr = status.add_functions();
    *func_ptr = function_name;
    status.set_utilization(0.10);

    kSchedulerPolicy->process_status(status);

    status.set_tid(2);
    status.set_utilization(0.90);

    kSchedulerPolicy->process_status(status);

    ThreadLocation loc = {status.ip(), status.tid()};

    EXPECT_EQ(kSchedulerPolicy->unpinned_cpu_executors.find(loc), kSchedulerPolicy->unpinned_cpu_executors.end());
    EXPECT_EQ(kSchedulerPolicy->function_locations[function_name].size(), 2);
    EXPECT_NE(kSchedulerPolicy->function_locations[function_name].find(loc), kSchedulerPolicy->function_locations[function_name].end());
    EXPECT_NE(kSchedulerPolicy->backoff.find(loc), kSchedulerPolicy->backoff.end());
}

TEST_F(PolicyTest, ProcessStatusRestart){
    string function_name = "square";
    ThreadStatus status;
    status.set_running(true);
    status.set_ip("127.0.0.1");
    status.set_tid(1);
    string* func_ptr = status.add_functions();
    *func_ptr = function_name;
    status.set_utilization(0);

    ThreadLocation loc = {status.ip(), status.tid()};

    // add metadata to the policy engine to make it think that this node
    // used to have a pinned function.
    kSchedulerPolicy->function_locations[function_name].clear();
    kSchedulerPolicy->function_locations[function_name].insert(loc);
    kSchedulerPolicy->thread_statuses[loc] = status;

    // Clear the status's pinned functions (i.e. restart)
    status.clear_functions();
    kSchedulerPolicy->process_status(status);

    EXPECT_EQ(kSchedulerPolicy->function_locations[function_name].size(), 0);
    EXPECT_NE(kSchedulerPolicy->unpinned_cpu_executors.find(loc), kSchedulerPolicy->unpinned_cpu_executors.end());
}

TEST_F(PolicyTest, ProcessStatusNotRunning){
    string function_name = "square";
    ThreadStatus status;
    status.set_running(true);
    status.set_ip("127.0.0.1");
    status.set_tid(1);
    string* func_ptr = status.add_functions();
    *func_ptr = function_name;
    status.set_utilization(0);

    ThreadLocation loc = {status.ip(), status.tid()};

    // add metadata to the policy engine to make it think that this node
    // used to have a pinned function.
    kSchedulerPolicy->function_locations[function_name].clear();
    kSchedulerPolicy->function_locations[function_name].insert(loc);
    kSchedulerPolicy->thread_statuses[loc] = status;

    // Clear the status' fields to report that it is turning off.
    status.Clear();
    status.set_running(false);
    status.set_ip("127.0.0.1");
    status.set_tid(1);

    kSchedulerPolicy->process_status(status);

    EXPECT_EQ(kSchedulerPolicy->thread_statuses.find(loc), kSchedulerPolicy->thread_statuses.end());
    EXPECT_EQ(kSchedulerPolicy->unpinned_cpu_executors.find(loc), kSchedulerPolicy->unpinned_cpu_executors.end());
    EXPECT_EQ(kSchedulerPolicy->function_locations[function_name].size(), 0);
}

TEST_F(PolicyTest, MetadataUpdate){
    string old_ip = "127.0.0.1";
    string new_ip = "192.168.0.1";
    ThreadLocation old_executor = {old_ip, 1};
    ThreadLocation new_executor = {new_ip, 2};

    ThreadStatus old_status;
    old_status.set_ip(old_ip);
    old_status.set_tid(1);
    old_status.set_running(true);

    ThreadStatus new_status;
    new_status.set_ip(new_ip);
    new_status.set_tid(2);
    new_status.set_running(true);

    kSchedulerPolicy->thread_statuses[old_executor] = old_status;
    kSchedulerPolicy->thread_statuses[new_executor] = new_status;

    // Add two executors, one with old an old backoff and one with a new time.
    std::chrono::system_clock::duration diff(std::chrono::seconds(10));
    kSchedulerPolicy->backoff[old_executor] = std::chrono::system_clock::now() - diff;
    kSchedulerPolicy->backoff[new_executor] = std::chrono::system_clock::now();

    kSchedulerPolicy->running_counts.clear();
    for(int i = 0; i < 10; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        kSchedulerPolicy->running_counts[new_executor].insert(
            get_time_since_epoch() - 10000);
    }

    for(int i = 0; i < 10; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        kSchedulerPolicy->running_counts[new_executor].insert(
            get_time_since_epoch());
    }

    StringSet old_set, new_set;
    for(string key : {"key1", "key2", "key3"}){
        *(old_set.add_keys()) = key;
    }
    for(string key : {"key3", "key4", "key5"}){
        *(new_set.add_keys()) = key;
    }
    string serialized_old, serialized_new;
    old_set.SerializeToString(&serialized_old);
    new_set.SerializeToString(&serialized_new);
    LWWPairLattice<string> payload_old(TimestampValuePair<string>(generate_timestamp(0), serialized_old));
    LWWPairLattice<string> payload_new(TimestampValuePair<string>(generate_timestamp(0), serialized_new));

    kvs_put(kvs_mock, get_cache_ip_key(old_ip), serialize(payload_old), log_, LatticeType::LWW);
    kvs_put(kvs_mock, get_cache_ip_key(new_ip), serialize(payload_new), log_, LatticeType::LWW);

    kSchedulerPolicy->update();

    // Check that the metadata has been correctly pruned.
    EXPECT_EQ(kSchedulerPolicy->backoff.size(), 1);
    EXPECT_NE(kSchedulerPolicy->backoff.find(new_executor), kSchedulerPolicy->backoff.end());
    EXPECT_EQ(kSchedulerPolicy->running_counts[new_executor].size(), 10);

    // Check that the caching information is correct.
    EXPECT_EQ(kSchedulerPolicy->key_locations["key1"].size(), 1);
    EXPECT_EQ(kSchedulerPolicy->key_locations["key2"].size(), 1);
    EXPECT_EQ(kSchedulerPolicy->key_locations["key3"].size(), 2);
    EXPECT_EQ(kSchedulerPolicy->key_locations["key4"].size(), 1);
    EXPECT_EQ(kSchedulerPolicy->key_locations["key5"].size(), 1);

    EXPECT_NE(kSchedulerPolicy->key_locations["key1"].find(old_ip),
    kSchedulerPolicy->key_locations["key1"].end());
    EXPECT_NE(kSchedulerPolicy->key_locations["key2"].find(old_ip),
    kSchedulerPolicy->key_locations["key2"].end());
    EXPECT_NE(kSchedulerPolicy->key_locations["key3"].find(old_ip),
    kSchedulerPolicy->key_locations["key3"].end());
    EXPECT_NE(kSchedulerPolicy->key_locations["key3"].find(new_ip),
    kSchedulerPolicy->key_locations["key3"].end());
    EXPECT_NE(kSchedulerPolicy->key_locations["key4"].find(new_ip),
    kSchedulerPolicy->key_locations["key4"].end());
    EXPECT_NE(kSchedulerPolicy->key_locations["key5"].find(new_ip),
    kSchedulerPolicy->key_locations["key5"].end());

}

TEST_F(PolicyTest, UpdateFunctionLocations){
    map<string, hset<ThreadLocation, pair_hash>> locations;
    string function1 = "square";
    string function2 = "increment";
    ThreadLocation loc1 = {"127.0.0.1", 0};
    ThreadLocation loc2 = {"127.0.0.2", 0};
    ThreadLocation loc3 = {"192.168.0.1", 0};

    locations[function1].insert(loc1);
    locations[function1].insert(loc3);
    locations[function2].insert(loc2);
    locations[function2].insert(loc3);

    SchedulerStatus status;
    for(auto& func_locations_pair : locations){
        string fname = func_locations_pair.first;
        hset<ThreadLocation, pair_hash> locs = func_locations_pair.second;
        for(ThreadLocation loc : locs){
            auto* loc_ptr = status.add_function_locations();
            loc_ptr->set_name(fname);
            loc_ptr->set_ip(loc.first);
            loc_ptr->set_tid(loc.second);
        }
    }

    kSchedulerPolicy->update_function_locations(status);

    EXPECT_EQ(kSchedulerPolicy->function_locations[function1].size(), 2);
    EXPECT_EQ(kSchedulerPolicy->function_locations[function2].size(), 2);

}



#endif //PROJECT_TEST_LIST_FUNCS_HPP
