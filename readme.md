phtree implementation based on 
Zäschke, Tilmann & Zimmerli, Christoph & Norrie, Moira. (2014). The PH-Tree - A Space-Efficient Storage Structure And Multi-Dimensional Index. Proceedings of the ACM SIGMOD International Conference on Management of Data. 397-408. 10.1145/2588555.2588564

Building:
  make directory "build"
  from build dir type command "cmake .."
  then type in terminal "make"

  to enable testing, uncomment in CMakeLists.txt
  -enable_testing
  -add_directory(test)

  to enable visualization, uncomment in CMakeLists.txt
  -add_directory(visualization)

  to enable benchmarking, uncomment in CMakeLists.txt
  -enable_testing
  -add_directory(bench)
  after running benchmarks you can see plots by invoking "python analyze.py" from "bench" directory
