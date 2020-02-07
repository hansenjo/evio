/**
 * Copyright (c) 2019, Jefferson Science Associates
 *
 * Thomas Jefferson National Accelerator Facility
 * Data Acquisition Group
 *
 * 12000, Jefferson Ave, Newport News, VA 23606
 * Phone : (757)-269-7100
 *
 * @date 04/10/2019
 * @author timmer
 */

#ifndef EVIO_6_0_EVIOEXCEPTION_H
#define EVIO_6_0_EVIOEXCEPTION_H

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <exception>


namespace evio {


class EvioException : public std::runtime_error {

public:

    explicit EvioException(const std::string & msg) noexcept : std::runtime_error(msg) {}
    explicit EvioException(const std::exception & ex) noexcept : std::runtime_error(ex.what()) {}

    EvioException(const std::string & msg, const char *file, int line) noexcept : std::runtime_error(msg) {
        std::ostringstream o;
        o << file << ":" << line << ":" << msg;
    }

};

#define throwEvioLine(arg) throw EvioException(arg, __FILE__, __LINE__);

}

#endif //EVIO_6_0_EVIOEXCEPTION_H
