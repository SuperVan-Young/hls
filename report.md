# HLS Project Report

薛晨皓
1900012982

---

The overall HLS algorithm goes as follows:

- Allocate resource types.
  Find a set of resource types covering all operation types.
  Allocate each operation type with a resource type.
- Schedule and bind.
  Search for a valid order and schedule one block at a time.
  The algorithm neglects resource constraint violation in this step.
- If previous binding does violate the area limit, cut down some resource
  instances until area constraint is met. Schedule and bind again.

## 1 Allocation

### 1.1 Allocate Resource Types

Initially, allocator selects a set of resources $\{r_i\} \sube R$ and allocate
one instance $r_i^{(0)}$ for each type.
The union of each resource type $r_i$'s compatible operation types $S_i$
should form a cover on the universe of operation types $S$,
 i.e, $\cup_i S_i \supe S$.
The total area should not exceed the given area limit.

The above statements could be formulated as an binary ILP problem, and be given
a solution with a linear programming solver, such as `lpsolve`.
To make better use of the LP-solver, we add two heuristics.
First, we set the optimization objective as maximizing the number of resources
$|\{ r_i\}|$, so that each operation type will have more resources to choose
from.
Second, w.r.t. the optimization objective, and the observation that
many useless resources appear in the resource library,
we simply fix their corresponding variables as zero to indicate discarding.

The preliminary resource type allocation lays a solid foundation
for the following steps.
It not only assures a valid solution,
but also shows flexibility under different scenarios.

### 1.2 Assign Operation Types with Resource Types

In this step, we assign each operation type $o_j$ with the best resource type
from $\{r_i\}$.
The selected resource type $r_i^*$ should introduce minimum overhead,
both temporal and spatial.
We use the following function to comprehensively
evaluate the overhead:

$$
\text{overhead} = \text{area} \times (\text{latency + 1}) / \theta,
$$

in which $\theta=2$ is a hyper-parameter to describe benefits from pipelining.

## 1.3 Cut Down Resource Instances

On the first run of scheduler and binder, they aren't aware of resource
constraints and assume infinite resource instances.
The result may violates the total area limit, and in that case,
we need to set a resource limit before trying again.

The allocator discards the most trivial resource instance, one at a time,
until the area constraints is met.
We roughly estimate the effect of deleting one resource instance with
the following formula:

$$
\text{induced latency} = \Delta E[\text{run times}] \times (\text{latency + 1}),
$$

Resource instance with minumum induced latency gets deleted, but we assures
that there's at least one resource instance of the same type remains.

## 2 Scheduling

### 2.1 Basic Block Ordering

We use BFS to search for a valid ordering of scheduling basic blocks.
BFS queue is initialized with the entry block.
For every basic block, check if all its prerequisite operations have been
executed (except for $\varphi$ nodes' inputs).
If the basic block is ready to schedule, mark all operations in it
as executed and push its successors to the BFS queue.

## 2.2 SDC Solver

We use an SDC solver to schedule one basic block.
We formulate the SDC problem in ILP and call `lpsolve` to solve it.
The optimization objective is to minimize the largest scheduled cycle.
The constraints are described in details in 2.3 and 2.4.

### 2.3 Dependency Constraints

For a pair of data-dependent operations in DFG, the successor must wait for
$\text{latency} + 1$ cycles after the predecessor.
In this way, successor fetches the data from registers,
which will not create complex combinational circuit,
and we won't bother removing false loops.

### 2.4 Resource Constraints

We add heuristics to avoid violating resource constraints.
For resource type $r_i$, we make a topology sorting on all the operation
assigned to it.
Assume we have $k$ instances of $r_i$, then $(k+j)$-th operation in the
topology sorting should not begin before $j$-th operation finishes.
Therefore, concurrent operations on $r_i$ will not exceed $k$.

## 3 Binding

We use left-edge algorithm to bind the operations.
First, we build a conflict graph, in which vertices are operations,
and edges indicates their execution are on the same resource type and
overlap temporally.
Next, we sort the operations with their scheduled cycle going ascendingly.
Finally, we use this ordering to color the conflict graph.
The index of a color represents which instance an operation uses.
