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

#include <stdlib.h>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "anna.pb.h"
#include "cloudburst.pb.h"


#include "scheduler_handler_base.hpp"
#include "test_connect_handler.hpp"
#include "test_function_create_handler.hpp"
#include "test_function_call_handler.hpp"
#include "test_dag_create_handler.hpp"

int main(int argc, char *argv[]) {
    log_->set_level(spdlog::level::info);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}