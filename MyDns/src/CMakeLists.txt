cmake_minimum_required(VERSION 3.21)
project(MyDns)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(LIBRARY_SOURCE dn2ip.c generate_new_id.c Get_TLD.c main.c Read_scheurl.c receiveDNS.c show.c)

add_library(dns SHARED ${LIBRARY_SOURCE})

include_directories(/home/yyx/MyDns/MyDns/src)

add_executable(MyDns
        dn2ip.c
        generate_new_id.c
        Get_TLD.c
        main.c
        Read_scheurl.c
        receiveDNS.c
        show.c
        total.h)

