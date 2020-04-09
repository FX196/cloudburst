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

#ifndef PROJECT_TEST_DAG_CALL_HANDLER_HPP
#define PROJECT_TEST_DAG_CALL_HANDLER_HPP

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

#endif //PROJECT_TEST_DAG_CALL_HANDLER_HPP
