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
    kSchedulerPolicy->unpinned_executors_.insert({ip, 1});
    kSchedulerPolicy->unpinned_executors_.insert({ip, 2});

    kSchedulerPolicy->backoff_[{ip, 1}] = std::chrono::system_clock::now();
    for(int i=0; i<1100; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        kSchedulerPolicy->running_counts_[{ip, 2}].insert(get_time_since_epoch());
    }

    ThreadLocation result = kSchedulerPolicy->pick_executor({}, "");
    EXPECT_EQ(result.first.empty(), true);
    EXPECT_EQ(result.second, 0);
}

TEST_F(PolicyTest, PinReject){
    string ip = "127.0.0.1";
    kSchedulerPolicy->unpinned_executors_.insert({ip, 1});
    kSchedulerPolicy->unpinned_executors_.insert({ip, 2});

    GenericResponse ok;
    ok.set_success(true);
    GenericResponse error;
    error.set_success(false);
    ((KvsMockClient *) kvs_mock)->responses_.push_back(ok);
    ((KvsMockClient *) kvs_mock)->responses_.push_back(error);

}



#endif //PROJECT_TEST_LIST_FUNCS_HPP
