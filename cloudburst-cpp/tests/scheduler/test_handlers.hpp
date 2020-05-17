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

#ifndef PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP
#define PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP

#include "scheduler/scheduler_handlers.hpp"

TEST_F(SchedulerHandlerTest, ConnectReceive){
vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 0);

string serialized = "";
zmq::socket_t connect_socket(context, ZMQ_REP);
string route_addr = "route_addr";

connect_handler(serialized, connect_socket, route_addr, log_);

messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);
EXPECT_EQ(messages[0], "route_addr");

std::cout << get_time_since_epoch() << std::endl;
}

TEST_F(SchedulerHandlerTest, FunctionCreateReceive){
    Function func;
    func.set_name("foo");
    func.set_body("foo body");
    string serialized;
    func.SerializeToString(&serialized);

    ((KvsMockClient *) kvs_mock)->responses_.push_back(get_func_list_response());

    zmq::socket_t create_socket(context, ZMQ_REP);

    function_create_handler(serialized, create_socket, kvs_mock, log_);

    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_get_.size(), 1);
    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_get_[0], FUNCOBJ);
    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_.size(), 2);
    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[0], FUNC_PREFIX + "foo");
    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[1], FUNCOBJ);

    vector<string> messages = get_zmq_messages();
    EXPECT_EQ(messages.size(), 1);
    string serialized_ok = messages[0];
    GenericResponse ok;
    ok.ParseFromString(serialized_ok);
    EXPECT_EQ(ok.success(), true);
}

TEST_F(SchedulerHandlerTest, FunctionCallSuccess){
    FunctionCall call;
    call.set_name("foo");
    string serialized_call;
    call.SerializeToString(&serialized_call);

    mock_policy.pick_executor_responses_.push_back(pair<Address, unsigned>("tcp://127.0.0.1:6000", 0));


    zmq::socket_t func_call_socket(context, ZMQ_REP);

    function_call_handler(serialized_call, func_call_socket, pusher_cache, kSchedulerPolicy, log_);

    vector<string> messages = get_zmq_messages();
    EXPECT_EQ(messages.size(), 2);

    FunctionCall received_call;
    received_call.ParseFromString(messages[0]);
    EXPECT_NE(received_call.response_key(), "");

    GenericResponse response;
    response.ParseFromString(messages[1]);
    EXPECT_EQ(response.success(), true);
    EXPECT_EQ(response.response_id(), received_call.response_key());
    }

    TEST_F(SchedulerHandlerTest, FunctionCallFail){
    FunctionCall call;
    call.set_name("foo");
    string serialized_call;
    call.SerializeToString(&serialized_call);

    mock_policy.pick_executor_responses_.push_back(pair<Address, unsigned>("", 0));


    zmq::socket_t func_call_socket(context, ZMQ_REP);

    function_call_handler(serialized_call, func_call_socket, pusher_cache, kSchedulerPolicy, log_);

    vector<string> messages = get_zmq_messages();
    EXPECT_EQ(messages.size(), 1);

    GenericResponse response;
    response.ParseFromString(messages[0]);
    EXPECT_EQ(response.success(), false);
}

TEST_F(SchedulerHandlerTest, DagCreateSuccess){
Function func_add;
func_add.set_name("add");
func_add.set_body("a+b");
string serialized_add;
func_add.SerializeToString(&serialized_add);
Function func_mul;
func_mul.set_name("mul");
func_mul.set_body("a*b");
string serialized_mul;
func_mul.SerializeToString(&serialized_mul);

Function func_mul2;
func_mul2.set_name("mul2");
func_mul2.set_body("a*b");
string serialized_mul2;
func_mul.SerializeToString(&serialized_mul2);

Dag dag;
dag.set_name("mul_mul_add");
Dag::FunctionReference* f1 = dag.add_functions();
f1->set_name("add");
f1->set_type(ExecutionType::REGULAR);
Dag::FunctionReference* f2 = dag.add_functions();
f2->set_name("mul");
f2->set_type(ExecutionType::REGULAR);
Dag::FunctionReference* f3 = dag.add_functions();
f3->set_name("mul2");
f3->set_type(ExecutionType::REGULAR);
Dag::Link* l1 = dag.add_connections();
l1->set_source("mul");
l1->set_sink("mul2");
Dag::Link* l2 = dag.add_connections();
l2->set_source("add");
l2->set_sink("mul2");

string serialized;
dag.SerializeToString(&serialized);


//mock_policy.pick_executor_responses_.push_back(pair<Address, unsigned>("tcp://127.0.0.1:6000", 0));
((KvsMockClient *) kvs_mock)->responses_.push_back(get_put_ok_response(0));

for(unsigned i=0; i<3; i++){
mock_policy.pin_function_responses_.push_back(true);
}

zmq::socket_t dag_create_socket(context, ZMQ_REP);

dag_create_handler(serialized, dag_create_socket, pusher_cache, kvs_mock, dags, kSchedulerPolicy, call_frequency, log_);

EXPECT_NE(dags.find("mul_mul_add"), dags.end());
EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[0], "mul_mul_add");
EXPECT_EQ(call_frequency.size(), 3);

vector<string> messages = get_zmq_messages();
GenericResponse response;
response.ParseFromString(messages[0]);
EXPECT_EQ(response.success(), true);

}

TEST_F(SchedulerHandlerTest, DagDeleteDNE){

string dag_name = "foo";

zmq::socket_t dag_delete_socket(context, ZMQ_REP);

dag_delete_handler(dag_name, dag_delete_socket, dags, kSchedulerPolicy, call_frequency, log_);

vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);
GenericResponse received_response;
received_response.ParseFromString(messages[0]);
EXPECT_EQ(received_response.success(), false);
EXPECT_EQ(received_response.error(), CloudburstError::NO_SUCH_DAG);
}

TEST_F(SchedulerHandlerTest, DagCallDagDNE){
DagCall call;
call.set_name("none");
string serialized_call;
call.SerializeToString(&serialized_call);

zmq::socket_t dag_call_socket(context, ZMQ_REP);

dag_call_handler(serialized_call, dag_call_socket, pusher_cache, last_arrivals, interarrivals, dags, kSchedulerPolicy, call_frequency, log_);

vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);
GenericResponse received_response;
received_response.ParseFromString(messages[0]);
EXPECT_EQ(received_response.success(), false);
EXPECT_EQ(received_response.error(), CloudburstError::NO_SUCH_DAG);
}

#endif //PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP
