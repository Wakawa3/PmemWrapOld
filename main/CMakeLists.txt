add_executable(a.out PmemKVS.c)

target_link_directories(a.out PRIVATE /home/satoshi/testlib/lib)

#target_link_directories(a.out PRIVATE /usr/local/lib/pmdk_debug)
target_link_libraries(a.out pmemwrap)
target_link_libraries(a.out pmemobj)
target_link_libraries(a.out pmem)