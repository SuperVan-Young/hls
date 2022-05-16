# HLS Project Report

薛晨皓
1900012982

---

To tackle the high-level synthesis problem, our approach sequentially takes 
three separate steps: allocation, scheduling and binding. 

## 1 Allocation
In this step, we allocate each type of operation with a type of resource, 
and instantiate some resource instances under the area limit. 

### 1.1 Allocate Resource Types
We first decide which type of resource to use for each type of operation.
Here we take an assumption. Assuming that each opeartion type `optype_i`
is designated with a resource type `rtype_j`, we instantiate an exclusive
resource instance `inst_ij`, that is, all operations of `optype_i` must be 
executed on `inst_ij` and `inst_ij` only executes operations of `optype_i`.
In other word, we do not consider reusing the same resource instance
for different types of compatible operation. 

Then, we use a dynamic programming algorithm to search for the "best"
resource type allocation under the area limit. A heuristic algorithm is
used to roughly estimate how well a resource type allocation will do.
Let's say we have a data flow graph, with operations of the same type
in a basic block as vertices and data dependencies as edges. The data flow
graph is a DAG, whose depth indicates the maximum latency and whose width
indicates opportunity with parallelism. Given the resource type's latency,
delay and other information, we could estimate the resulting performance
of choosing resource type `rtype` for operation type `optype`.

### 1.2 Allocate Resource Instances

We make use of the leftover area to boost the performance. We follow the
assumption and heuristic algorithm in the previous section. Here we take
a greedy algorithm, that maintain a priority queue, the element of which
is to add one more instance for an operation type. Before we use up all
area, we choose to add an instance that will bring out the most expected
performance gaining with the least area cost.

## 2 Scheduling

We use a SDC solver and schedule the basic blocks one by one.

### 2.1 The order of basic blocks
We use BFS to find a valid ordering of basic blocks. Starting from the entry
block, we examine each basic blocks successors, and push them to bfs queue
if all operation it depends on have been scheduled (except for inputs of
phi nodes). 

### 2.2 Dependency Constraints
The most important part of SDC Scheduler is defining the dependencies
constraints. Let's say operation $i/j$ is scheduled to cycle $x_i \leq x_j$,
in which operation $j$ depends on the result of operation $i$.
If operation $i$ is executed on an non-pipelined resource instance $r$, 
$x_j - x_i \geq lat_r + 1$. 

Notice that we don't allow resource chaining here, which may lead to
performance degradation.

### 2.3 Resource Constraints
In order to prevent binding conflict, we add heuristic resource constraints
here. We use bfs get a topology ordering of operations of the same type
in a basic block. Suppose we have $k$ instances for `optype`, we add latency
constraints between $op_{i+k}$ and $op_{i}$. Pipelined resources could have
looser constaints: $x_{i+k} - x_{i} \geq 1$. 
Non-pipelined resources still cannot do chaining, 
so $x_{i+k} - x_{i} \geq lat_r + 1$.

## 3 Binding
We build a conflict graph on the given scheduling. Two operations are in
conflict if their execution overlaps. We use left-edge algorithm to get
a PEO and color all operations with the PEO. We could simply use the color
as the instance index it binds to, since each operation type exclusively 
use the instances they are allocated with.

## 4 Some Comments?
- I intended to replace heuristic resource constraints with a SAT solver,
  and build a SAT-SDC joint scheduling. However, I build my SDC solver 
  with `lpsolve`, which could not give the wronged negative loop.
  Therefore, my SAT solver couldn't get any feedback! Duh.
- I also wanted to allow resource chaining. In this case, I would have to 
  resolve chaining conflicts, and use Chain-Annotated Compatibility Graph
  to get a more sophisticated conflict graph. However, coloring on this
  conflict graph may violate the resource limit, and I have to go over 
  everything again, which is not friendly to the current program structure.
- Sharing resources with compatible operations seems beneficial, but I 
  observed that allocating more resource instances barely boosts the 
  performance. So I gave up this idea eventually.