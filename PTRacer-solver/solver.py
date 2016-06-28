#! /usr/bin/env python

import argparse, os
import util
from Operation import *
from z3 import *

parser = argparse.ArgumentParser(description='Solve constraints to check branch feasibilty')
parser.add_argument('-t', type=str, help='path/to/traces/folder')

args = parser.parse_args()
print(args)

num_consts = 0

traces = {}
branch_checks = []

opersByTask = {}
readSet = {}
writeSet = {}
lockPairSet = {}
lockPairStack = {}
branchCheckOpers = []
spawnOpers = []

compOpers = ["", "==", "!=", ">", "<", ">=", "<="]

formula_inits = {}
read_inits = {}

const_s = Solver()

def create_f_inits():
    for tid in opersByTask:
        taskOpers = opersByTask[tid]
        for oper in taskOpers:
            #print oper.taskId
            #print oper.var
            #print oper.id
            var_name = ""
            if isinstance(oper, LockPairOperation):
                if oper.unlockId == 0:
                    var_name = "L@" + str(oper.taskId) + str(oper.id)
                else:
                    var_name = "U@" + str(oper.taskId) + str(oper.unlockId)
                

            elif isinstance(oper, RWOperation):
                if oper.iswrite == True:
                    var_name = "W@" + str(oper.taskId) + str(oper.id)
                else:
                    var_name = "R@" + str(oper.taskId) + str(oper.id)                
                
            #print var_name
            formula_init = Int(var_name)

            # if tid in formula_inits:
            #     formula_inits[tid].append(formula_init)
            # else:
            #     list = [formula_init]
            #     formula_inits[tid] = list

            if tid in formula_inits:
                t_formula_init = formula_inits[tid]
                if var_name.startswith('U'):
                    t_formula_init[str(oper.unlockId)] = formula_init
                else:
                    t_formula_init[str(oper.id)] = formula_init
            else:
                t_formula_init = {}
                if var_name.startswith('U'):
                    t_formula_init[str(oper.unlockId)] = formula_init
                else:
                    t_formula_init[str(oper.id)] = formula_init
                formula_inits[tid] = t_formula_init

    #print formula_inits

def create_mo_consts():
    global num_consts
    for tid in formula_inits:
        index = 0
        task_f_inits = formula_inits[tid]

        index = 0
        prev = ""
        for t_i in task_f_inits:
            if index == 0:
                prev = t_i
                continue
            const_s.add(task_f_inits[prev] < task_f_inits[t_i])
            num_consts = num_consts + 1
            prev = t_i

def create_rw_consts():
    global num_consts
    outer_formula = True
    for var_name in readSet:
        read_opers = readSet[var_name]
        for read_oper in read_opers:
            const_name = "R" + str(read_oper.var) + "@" + str(read_oper.taskId) + str(read_oper.id)
            read_init = Int(const_name)
            #print read_init, read_oper.taskId, read_oper.id
            read_task_init = formula_inits[int(read_oper.taskId)]
            read_const = read_task_init[str(read_oper.id)]            

            if str(read_oper.taskId) in read_inits:
                t_read_init = read_inits[str(read_oper.taskId)]
                t_read_init[str(read_oper.id)] = read_init
            else:
                t_read_init = {}
                t_read_init[str(read_oper.id)] = read_init
                read_inits[str(read_oper.taskId)] = t_read_init
                
            middle_formula = False
            
            write_opers = writeSet[var_name]
            for write_oper in write_opers:
                write_task_init = formula_inits[int(write_oper.taskId)]
                write_const = write_task_init[str(write_oper.id)]
                inner_formula = And(read_init == int(write_oper.value), write_const < read_const)
                num_consts = num_consts + 3

                for other_write_oper in write_opers:
                    if not other_write_oper == write_oper:
                        o_write_task_init = formula_inits[int(other_write_oper.taskId)]
                        other_write_const = o_write_task_init[str(other_write_oper.id)]
                        inner_formula = And(inner_formula, Or(other_write_const < write_const, read_const < other_write_const))
                        num_consts = num_consts + 4
                #print inner_formula
                #const_s.add(inner_formula)
                middle_formula = Or(middle_formula, inner_formula)
                num_consts = num_consts + 1
        outer_formula = And(outer_formula, middle_formula)
        num_consts = num_consts + 1
    const_s.add(outer_formula)

def create_sync_consts():
    global num_consts
    outer_formula = True
    for lock in lockPairSet:
        lockPairs = lockPairSet[lock]
        middle_formula = False
        for lockPair in lockPairs:
            unlock_task_const = formula_inits[int(lockPair.taskId)]
            #print unlock_task_const, str(lockPair.unlockId)
            unlock_const = unlock_task_const[str(lockPair.unlockId)]
            #inner_formula = False
            for lockPair_inner in lockPairs:
                if lockPair == lockPair_inner:
                    continue
                lock_task_const = formula_inits[int(lockPair_inner.taskId)]
                lock_const = lock_task_const[str(lockPair_inner.id)]
                middle_formula = Or(middle_formula, unlock_const < lock_const)
                num_consts = num_consts + 2
    outer_formula = And(outer_formula, middle_formula)
    num_consts = num_consts + 1
    const_s.add(outer_formula)
            
def create_branch_consts():
    global num_consts
    for br_check in branchCheckOpers:
        task_read_inits = read_inits[str(br_check.taskId)]
        print task_read_inits
        print("BR_CHECKID = " + str(br_check.id))
        read_init = task_read_inits[str(br_check.id)]
        if br_check.compOper == CompOperEnum.eq:
            const_s.add(read_init == int(br_check.value))
        if br_check.compOper == CompOperEnum.neq:
            const_s.add(read_init != int(br_check.value))
        if br_check.compOper == CompOperEnum.lt:
            const_s.add(read_init < int(br_check.value))
        if br_check.compOper == CompOperEnum.gt:
            const_s.add(read_init > int(br_check.value))
        if br_check.compOper == CompOperEnum.lte:
            const_s.add(read_init <= int(br_check.value))
        if br_check.compOper == CompOperEnum.gte:
            const_s.add(read_init >= int(br_check.value))
        num_consts = num_consts + 1

def create_spsync_consts():
    global num_consts
    for spOper in spawnOpers:
        sp_task_init = formula_inits[int(spOper.taskId)]
        last_const = sp_task_init[str(spOper.lastOperId)]

        child_task_init = formula_inits[int(spOper.childId)]
        first_const = child_task_init.values()[0]

        const_s.add(last_const < first_const)
        num_consts = num_consts + 1

def create_check_formula():
    create_f_inits()
    create_mo_consts()
    create_rw_consts()
    create_sync_consts()
    create_branch_consts()
    create_spsync_consts()
    
    print const_s

    sat_check = ""
    sat_check = str(const_s.check())
    print(const_s.check())
    util.chdir(ref_cwd)
    f = open('sat_check.out', 'w')
    f.write(sat_check + "\n")
    #print(const_s.model())

def output_stats():
    num_variables = 0
    for tid in formula_inits:
        task_f_inits = formula_inits[tid]
        num_variables = num_variables + len(task_f_inits)

    for tid in read_inits:
        task_r_inits = read_inits[tid]
        num_variables = num_variables + len(task_r_inits)

    print "Number of variables = " + str(num_variables)
    print "Number of constraints = " + str(num_consts)


def create_operations():
    for tid in traces:
        trace = traces[tid]
        for oper in trace:
            if oper.startswith('R'):
                readOper = RWOperation()
                readOper.taskId = tid
                readOper.id = oper[oper.index('.')+1:]
                var_name = oper[1:oper.index('@')]
                readOper.var = var_name
                if var_name in readSet:
                    readSet[var_name].append(readOper)
                else:
                    list = [readOper]
                    readSet[var_name] = list
                    
                if tid in opersByTask:
                    opersByTask[tid].append(readOper)
                else:
                    list = [readOper]
                    opersByTask[tid] = list
                
            elif oper.startswith('W'):
                wrOper = RWOperation()
                wrOper.taskId = tid
                var_name = oper[1:oper.index('@')]
                wrOper.var = var_name
                wrOper.iswrite = True
                wrOper.id = oper[oper.index('.')+1:oper.index('=')]
                wrOper.value = oper[oper.index('=') + 1:]

                if var_name in writeSet:
                    writeSet[var_name].append(wrOper)
                else:
                    list = [wrOper]
                    writeSet[var_name] = list

                if tid in opersByTask:
                    opersByTask[tid].append(wrOper)
                else:
                    list = [wrOper]
                    opersByTask[tid] = list

            elif oper.startswith('L'):
                lockOper = LockPairOperation()
                lockOper.taskId = tid
                var_name = oper[1:oper.index('@')]
                lockOper.var = var_name
                lockOper.id = oper[oper.index('.')+1:]
                if var_name in lockPairStack:
                    lockPairStack[var_name].append(lockOper)
                else:
                    list = [lockOper]
                    lockPairStack[var_name] = list

                if tid in opersByTask:
                    opersByTask[tid].append(lockOper)
                else:
                    list = [lockOper]
                    opersByTask[tid] = list

            elif oper.startswith('U'):
                var_name = oper[1:oper.index('@')]
                lockOper = lockPairStack[var_name].pop()
                newLockOper = LockPairOperation()
                newLockOper.taskId = lockOper.taskId
                newLockOper.var = lockOper.var
                newLockOper.id = lockOper.id
                newLockOper.unlockId = oper[oper.index('.')+1:]

                if var_name in lockPairSet:
                    lockPairSet[var_name].append(newLockOper)
                else:
                    list = [newLockOper]
                    lockPairSet[var_name] = list

                if tid in opersByTask:
                    opersByTask[tid].append(newLockOper)
                else:
                    list = [newLockOper]
                    opersByTask[tid] = list
            elif oper.startswith('S'):
                if tid not in opersByTask:
                    continue
                spOper = SpawnOperation()
                spOper.taskId = tid
                spOper.id = oper[oper.index('.')+1:oper.index('=')]
                spOper.childId = oper[oper.index('=') + 1:]
                for i_oper in reversed(opersByTask[tid]):
                    if isinstance(i_oper, SpawnOperation):
                        continue;
                    spOper.lastOperId = i_oper.id
                #opersByTask[tid].append(spOper)
                spawnOpers.append(spOper)

                    
    for br_check in branch_checks:
        br_oper = BranchCheckOperation()
        br_oper.var = br_check[1:br_check.index('@')]
        br_oper.taskId = br_check[br_check.index('@')+1:br_check.index('.')]
        
        compOper = br_check[br_check.index(' ')+1:br_check.rindex(' ')]
        #print compOper
        br_oper.compOper = compOpers.index(compOper)
        #print br_oper.compOper
        br_oper.id = br_check[br_check.index('.')+1:br_check.index(compOper)-1]
        if br_oper.compOper == CompOperEnum.gt or br_oper.compOper == CompOperEnum.lt:
            br_oper.value = br_check[br_check.index(compOper) + 2:]
        else:
            br_oper.value = br_check[br_check.index(compOper) + 3:]
        #print br_oper.value
        branchCheckOpers.append(br_oper)

def parse_traces(path):
    util.chdir(path)
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    for filename in files:
        if filename.startswith('Trace_'):
            file = open(filename, 'r')
            lines = file.read().splitlines()
            und_index = filename.index('_');
            task_id = filename[und_index+1:]
            traces[int(task_id)] = lines

        if filename.startswith('br_'):
            file = open(filename, 'r')
            lines = file.read().splitlines()
            branch_checks.extend(lines)

ref_cwd = os.getcwd();
parse_traces(args.t)
#print traces
#print branch_checks

create_operations()
#print opersByTask
#print readSet
#print writeSet
#print lockPairSet
#print branchCheckOpers

create_check_formula()
output_stats()

#x = Real('x')
#y = Real('y')
#s = Solver()
#s.add(x + y > 5, x > 1, y > 1)
#s.add(x == 5)
# s.add(x < 1)
# s.add(y < 1)
#print s
#print(s.check())
# #print(s.model())
