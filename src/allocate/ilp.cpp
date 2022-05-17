#include "ilp.h"

namespace hls {

const float theta_pipeline = 2.0;

// Use ILP to allocate resource types.
// Assume all resource types are instantiated once, under the given area limit,
// find the set with maximum size that could cover all operation types.
int ILPAllocator::allocate_resource_type() {
    int *colno = new int[hin->n_resource_type];  // ordered by bb.ops
    REAL *row = new REAL[hin->n_resource_type];
    int ret = 0;  // if ret == -1, will skip to cleaning up

    // build an 0-1 ILP model
    auto lp = make_lp(0, hin->n_resource_type);
    if (lp == 0) ret = -1;
    if (!ret) {
        set_add_rowmode(lp, TRUE);
        for (int i = 1; i <= hin->n_resource_type; i++) {
            set_int(lp, i, TRUE);
            colno[0] = i;
            row[0] = 1;
            if (!add_constraintex(lp, 1, row, colno, LE, 1)) {
                cerr << "Error: adding variant upper bound" << endl;
                ret = - -1;
            }
        }
    }

    // add operation type constraints
    if (!ret) {
        for (int ot = 0; ot < hin->n_op_type; ot++) {
            // load & store needs scheduling, adding memory resource.
            if (!hin->need_schedule(hin->op_types[ot])) continue;
            int cnt = 0;
            for (auto rtid : ot2comprt[ot]) {
                // must have one resource type to use
                colno[cnt] = rtid + 1;
                row[cnt] = 1;
                cnt++;
            }
            if (!add_constraintex(lp, cnt, row, colno, GE, 1)) {
                cerr << "Error: adding operation type constraints" << endl;
                ret = -1;
            }
        }
    }

    // add area constraints
    if (!ret) {
        int cnt = 0;
        for (const auto &rt : hin->resource_types) {
            row[cnt] = rt.area;
            cnt++;
        }
        if (!add_constraint(lp, row, LE, hin->area_limit)) {
            cerr << "Error: adding area constraints" << endl;
            ret = -1;
        }
    }

    // set objective: maximize number of available resource types
    if (!ret) {
        set_add_rowmode(lp, FALSE);
        set_maxim(lp);
        for (int i = 0; i < hin->n_resource_type; i++) row[i] = 1;
        if (!set_obj_fn(lp, row)) {
            cerr << "Error: setting objective functions" << endl;
            ret = -1;
        }
    }

    // run the model and fetch the result
    if (!ret) {
        int ret_lp = solve(lp);
        if (!(ret_lp == OPTIMAL || ret_lp == SUBOPTIMAL)) {
            cerr << "LP fails with return value = " << ret_lp << endl;
            ret = -1;
        }
        get_variables(lp, row);
        for (int i = 0; i < hin->n_resource_type; i++) {
            rtypes[i] = ((int)row[i] == 1);
        }
    }

    // cleaning up
    if (lp) delete_lp(lp);
    delete[] colno;
    delete[] row;
    return ret;
}

// Allocate each operation type with the best resource type
int ILPAllocator::allocate_operation_type() {
    for (int ot = 0; ot < hin->n_op_type; ot++) {
        if (!hin->need_schedule(hin->op_types[ot])) continue;

        float perf = std::numeric_limits<float>::infinity();
        for (auto rtid : ot2comprt[ot]) {
            // We didn't choose this kind of resource type
            if (!rtypes[rtid]) continue;

            // select this resource type if it's better
            const auto &rt = hin->resource_types[rtid];
            float now_perf = (rt.area * (rt.latency + 1)) / theta_pipeline;
            if (now_perf < perf) {
                perf = now_perf;
                ot2rtid[ot] = rtid;
            }
        }

        // Check the result
        if (ot2rtid[ot] == -1) {
            cerr << "Error: optype " << ot << " didn't get allocated!" << endl;
            return -1;
        }
    }
    return 0;
}

typedef struct {
    float exp_time;
    const ResourceType *rt;
    int num;

    bool operator>(const QueueNode &x) const {
        auto res = (exp_time * (rt->latency + 1)) / (num * (num - 1));
        auto res_ = (x.exp_time * (x.rt->latency + 1)) / (x.num * (x.num - 1));
        return res > res_;
    }
} QueueNode;

int ILPAllocator::allocate_insts_bound(vector<int> &rinsts) {
    // get current total area used
    int total_area = 0;
    for (int rtid = 0; rtid < hin->n_resource_type; rtid++) {
        const auto &rt = hin->resource_types[rtid];
        total_area += rinsts[rtid] * rt.area;
    }

    // Get expected times of execution on rtype
    vector<float> exp_times(hin->n_resource_type, 0.0);
    for (const auto &op : hin->operations) {
        int rtid = ot2rtid[op.optype];
        if (rtid == -1) continue;
        const auto &bb = hin->blocks[op.opid];
        exp_times[rtid] += bb.exp_times;
    }

    // Cut down relatively useless resource instance (greater for minimum heap)
    // Make sure no node in q has num <= 1
    priority_queue<QueueNode, vector<QueueNode>, std::greater<QueueNode>> q;
    for (int rtid = 0; rtid < hin->n_resource_type; rtid++) {
        const auto &rt = hin->resource_types[rtid];
        if (rinsts[rtid] > 1) {
            QueueNode n;
            n.exp_time = exp_times[rtid];
            n.num = rinsts[rtid];
            n.rt = &rt;
            q.push(n);
        }
    }

    while (total_area > hin->area_limit) {
        auto n = q.top();
        q.pop();

        n.num = --rinsts[n.rt->rtid];
        total_area -= n.rt->area;

        if (n.num > 1) {
            q.push(n);
        }
    }

    return 0;
}

}  // namespace hls
