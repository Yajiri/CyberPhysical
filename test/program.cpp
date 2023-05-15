#include <iostream>
#include "../lib/cluon-complete-v0.0.127.hpp"

int main(int argc, char **argv) {
    cluon::data::TimeStamp stamp = cluon::time::now();
    std::cout << stamp.microseconds() << std::endl;
    return 0;
}
