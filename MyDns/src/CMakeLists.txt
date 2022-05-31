cmake_minimum_required(VERSION 3.21)
project(MyDns)

set(CMAKE_CXX_STANDARD 14)
set(LIBRARY_SOURCE dn2ip.cpp generate_new_id.cpp Get_TLD.cpp main.cpp Read_scheurl.cpp receiveDNS.cpp show.cpp)

add_library(dns SHARED ${LIBRARY_SOURCE})

include_directories(/home/yyx/MyDns/MyDns/src)

add_executable(MyDns
        dn2ip.cpp
        generate_new_id.cpp
        Get_TLD.cpp
        main.cpp
        Read_scheurl.cpp
        receiveDNS.cpp
        show.cpp
        total.h)

