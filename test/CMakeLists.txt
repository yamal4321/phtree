cmake_minimum_required(VERSION 3.0)

set (FLAG "-O3")

set (Ds 1 2 4 8)
set (Hs 1 2 4 8 16 32 64 128)

set (TESTS "test_bitstring" "test_encode" "test_node" "test_mask" "test_tree" "test_tree_parallel" "test_query" "test_knn")

foreach(T ${TESTS})
  foreach(D ${Ds})
    foreach(H ${Hs})
      configure_file(${T}.cpp ${T}${D}_${H}.cpp)
      add_executable(${T}${D}_${H} ${T}${D}_${H}.cpp)
      #target_compile_options(${T}${D}_${H} PRIVATE -march=native ${FLAG} -fsanitize=undefined -fsanitize=address) //broken with ASLR in current kernel
      #target_link_options(${T}${D}_${H} PRIVATE -fsanitize=undefined -fsanitize=address)
      add_test(NAME ${T}_${D}_${H} COMMAND ${T}${D}_${H})
    endforeach()
  endforeach()
endforeach()
