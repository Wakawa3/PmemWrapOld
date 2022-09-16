add_executable(a.out PmemKVS.c)
# a.outをコンパイルする際にgreetingsをリンクする
target_link_libraries(a.out pmemwrap)
target_link_libraries(a.out pmem)
target_link_libraries(a.out pmemobj)