#include <iostream>

#include "scheduler/scheduler_handlers.hpp"
ZmqUtil zmq_util;
ZmqUtilInterface *kZmqUtil = &zmq_util;

int main(int argc, char const *argv[]) {
    GenericResponse error;
    std::cout << "Hello world.";
    return 0;
}
