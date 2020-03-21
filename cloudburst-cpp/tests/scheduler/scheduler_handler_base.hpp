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

#ifndef PROJECT_SCHEDULER_HANDLER_BASE_HPP
#define PROJECT_SCHEDULER_HANDLER_BASE_HPP

#include "mock_zmq_utils.hpp"
#include "scheduler/policy/mock_scheduler_policy.hpp"

MockZmqUtil mock_zmq_util;
ZmqUtilInterface *kZmqUtil = &mock_zmq_util;

MockSchedulerPolicy policy;

logger log_ = spdlog::basic_logger_mt("mock_log", "mock_log.txt", true);

class SchedulerHandlerTest : public ::testing::Test {
    // initialization
};

#endif //PROJECT_SCHEDULER_HANDLER_BASE_HPP
