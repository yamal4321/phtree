# PH-Tree implementation based on paper
Zäschke, Tilmann & Zimmerli, Christoph & Norrie, Moira. (2014). The PH-Tree - A Space-Efficient Storage Structure And Multi-Dimensional Index. Proceedings of the ACM SIGMOD International Conference on Management of Data. 397-408. 10.1145/2588555.2588564.

# Used 3rd party libraries
- https://github.com/Forceflow/libmorton

# Benchmarks
x-axis: tree depth/number of entries <br>
y-axis: dimensions/time (ns) per returned entry

**point query**
  <img src="images/point_query.png">
**insert**
  <img src="images/insert.png">
**remove**
  <img src="images/remove.png">
**rect intersect**
  <img src="images/intersect_query.png">
**knn query**
  <img src="images/knn_query.png">

# Limitations
max dimensions = 32 <br>
dimensions should be power of 2
