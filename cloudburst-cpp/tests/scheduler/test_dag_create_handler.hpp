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

#ifndef PROJECT_TEST_DAG_CREATE_HANDLER_HPP
#define PROJECT_TEST_DAG_CREATE_HANDLER_HPP

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
    string* f1 = dag.add_functions();
    *f1 = "add";
    string* f2 = dag.add_functions();
    *f2 = "mul";
    string* f3 = dag.add_functions();
    *f3 = "mul2";
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

    dag_create_handler(serialized, dag_create_socket, pusher_cache, kvs_mock, dags, mock_policy, call_frequency, log_);

    EXPECT_NE(dags.find("mul_mul_add"), dags.end());
    EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[0], "mul_mul_add");
    EXPECT_EQ(call_frequency.size(), 3);

    vector<string> messages = get_zmq_messages();
    GenericResponse response;
    response.ParseFromString(messages[0]);
    EXPECT_EQ(response.success(), true);

}

#endif //PROJECT_TEST_DAG_CREATE_HANDLER_HPP
