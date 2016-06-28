/*
    Copyright 2005-2014 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks. Threading Building Blocks is free software;
    you can redistribute it and/or modify it under the terms of the GNU General Public License
    version 2  as  published  by  the  Free Software Foundation.  Threading Building Blocks is
    distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See  the GNU General Public License for more details.   You should have received a copy of
    the  GNU General Public License along with Threading Building Blocks; if not, write to the
    Free Software Foundation, Inc.,  51 Franklin St,  Fifth Floor,  Boston,  MA 02110-1301 USA

    As a special exception,  you may use this file  as part of a free software library without
    restriction.  Specifically,  if other files instantiate templates  or use macros or inline
    functions from this file, or you compile this file and link it with other files to produce
    an executable,  this file does not by itself cause the resulting executable to be covered
    by the GNU General Public License. This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General Public License.
*/

#include "common.h"
#include "tbb/t_debug_task.h"
#include "tbb/exec_calls.h"
#include <iostream>

class SimpleSumTask: public tbb::t_debug_task {
    Value* const sum;
    TreeNode* root;
public:
    SimpleSumTask( TreeNode* root_, Value* sum_ ) : root(root_), sum(sum_) {}
    task* execute() {
      __exec_begin__(getTaskId());
      //std::cout << "***Count = " << root->node_count << std::endl;
        if( root->node_count<1000 ) {
	  //std::cout << "***SERIALSUM***\n";
            *sum = SerialSumTree(root);
        } else {
            Value x, y;
            int count = 1;
	    if( root->left ) {
	      ++count;
	    }
	    if( root->right ) {
	      count++;
	    }
	    set_ref_count(count);
            if( root->left ) {
	      //std::cout << "***SPAWN LEFT***" << std::endl;
	      //std::cout << "*****Left Count = " << root->left->node_count << std::endl;
		tbb::t_debug_task& a = *new( allocate_child() ) SimpleSumTask(root->left,&x);
		tbb::t_debug_task::spawn(a);
		//std::cout << "Task Id = " << a.getTaskId() << std::endl;
	    }
	    if( root->right ) {
	      //std::cout << "***SPAWN RIGHT***" << std::endl;
	      //std::cout << "*****Right Count = " << root->right->node_count << std::endl;
	      tbb::t_debug_task& b = *new( allocate_child() ) SimpleSumTask(root->right,&y);
	      tbb::t_debug_task::spawn(b);
	      //std::cout << "Task Id = " << b.getTaskId() << std::endl;
	    }
            // Argument to set_ref_count is one more than size of the list,
            // because spawn_and_wait_for_all expects an augmented ref_count.
	    //set_ref_count(count);
	    tbb::task::wait_for_all();
            *sum = root->value;
            if( root->left ) *sum += x;
            if( root->right ) *sum += y;
        }
	__exec_end__(getTaskId());
	//__exec_end__(0);
        return NULL;
    }
};

Value SimpleParallelSumTree( TreeNode* root ) {
    Value sum;
    SimpleSumTask& a = *new(tbb::task::allocate_root()) SimpleSumTask(root,&sum);
    tbb::t_debug_task::spawn_root_and_wait(a);
    //std::cout << "Root taskid = " << a.getTaskId() << std::endl;
    return sum;
}

