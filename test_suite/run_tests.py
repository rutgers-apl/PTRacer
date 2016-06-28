#! /usr/bin/python

import sys, string, os, popen2, shutil, platform, argparse
import util

parser = argparse.ArgumentParser(description='Script to run test cases')
parser.add_argument('-d', type=str, default='ptracer', help='Set to ptracer to run PTRacer or spd3 to run SPD3. Default ptracer.')

args = parser.parse_args()

if args.d == 'ptracer' or args.d == 'spd3':
    print(args)
else:
    print(args)
    print "-d argument should be ptracer or spd3"
    sys.exit()

#clean up the obj directories
do_clean = False

#build the configs
do_build = False

#run the tests
do_run = True

if do_clean and not do_build:
    print "Clean - true and build - false not allowed"
    exit

#set paths
TBBROOT=os.environ['TBBROOT']
print "TBBROOT = " + TBBROOT
TD_ROOT=os.environ['TD_ROOT']
print "TD_ROOT = " + TD_ROOT
SOLVER_ROOT=os.environ['SOLVER_ROOT']
print "SOLVER_ROOT = " + SOLVER_ROOT

configs = []

entry = { "NAME" : "dr_detector",
          "BUILD_LINE" : "make",
          "CLEAN_LINE" : " make clean ",
          "BUILD_ARCH" : "x86_64",
          "RUN_ARCH" : "x86_64",
          "ADDITIONAL_FLAGS" : "",
          "RUN_LINE" : "./",
          "ARGS" : "",
          }

configs.append(entry);

benchmarks=[
    "dr_12_Yes",
    "dr_13_Yes",
    "dr_14_Yes",
    "dr_15_Yes",
    "dr_16_Yes",
    "dr_17_Yes",
    "dr_18_Yes",
    "dr_19_Yes",
    "dr_110_Yes",
    "dr_111_Yes",
    "dr_112_Yes",
    "dr_113_Yes",
    "dr_114_Yes",
    "dr_115_Yes",
    "dr_116_Yes",
    "dr_117_Yes",
    "dr_118_Yes",
    "dr_119_Yes",
    "dr_120_Yes",
    "dr_121_Yes",
    "dr_12_Yes_fixed",
    "dr_13_Yes_fixed",
    "dr_14_Yes_fixed",
    "dr_15_Yes_fixed",
    "dr_16_Yes_fixed",
    "dr_17_Yes_fixed",
    "dr_18_Yes_fixed",
    "dr_19_Yes_fixed",
    "dr_110_Yes_fixed",
    "dr_111_Yes_fixed",
    "dr_112_Yes_fixed",
    "dr_113_Yes_fixed",
    "dr_114_Yes_fixed",
    "dr_115_Yes_fixed",
    "dr_116_Yes_fixed",
    "dr_117_Yes_fixed",
    "dr_118_Yes_fixed",
    "dr_119_Yes_fixed",
    "dr_120_Yes_fixed",
    "dr_121_Yes_fixed",
    "dr_serial_12_Yes_fixed",
    "dr_serial_13_Yes_fixed",
    "dr_serial_14_Yes_fixed",
    "dr_serial_15_Yes_fixed",
    "dr_serial_16_Yes_fixed",
    "dr_serial_17_Yes_fixed",
    "dr_serial_18_Yes_fixed",
    "dr_serial_19_Yes_fixed",
    "dr_serial_110_Yes_fixed",
    "dr_serial_111_Yes_fixed",
    "dr_serial_112_Yes_fixed",
    "dr_serial_113_Yes_fixed",
    "dr_serial_114_Yes_fixed",
    "dr_serial_115_Yes_fixed",
    "dr_serial_116_Yes_fixed",
    "dr_serial_117_Yes_fixed",
    "dr_serial_118_Yes_fixed",
    "dr_serial_119_Yes_fixed",
    "dr_serial_120_Yes_fixed",
    "dr_serial_121_Yes_fixed",
    "dr_lock_12_Yes",
    "dr_lock_13_Yes",
    "dr_lock_14_Yes",
    "dr_lock_15_Yes",
    "dr_lock_16_Yes",
    "dr_lock_17_Yes",
    "dr_lock_18_Yes",
    "dr_lock_19_Yes",
    "dr_lock_110_Yes",
    "dr_lock_111_Yes",
    "dr_lock_112_Yes",
    "dr_lock_113_Yes",
    "dr_lock_114_Yes",
    "dr_lock_115_Yes",
    "dr_lock_116_Yes",
    "dr_lock_117_Yes",
    "dr_lock_118_Yes",
    "dr_lock_119_Yes",
    "dr_lock_120_Yes",
    "dr_lock_121_Yes",
    "dr_branch_12_Yes",
    "dr_branch_13_Yes",
    "dr_branch_14_Yes",
    "dr_branch_15_Yes",
    "dr_branch_16_Yes",
    "dr_branch_17_Yes",
    "dr_branch_18_Yes",
    "dr_branch_19_Yes",
    "dr_branch_110_Yes",
    "dr_branch_111_Yes",
    "dr_branch_112_Yes",
    "dr_branch_113_Yes",
    "dr_branch_114_Yes",
    "dr_branch_115_Yes",
    "dr_branch_116_Yes",
    "dr_branch_117_Yes",
    "dr_branch_118_Yes",
    "dr_branch_119_Yes",
    "dr_branch_120_Yes",
    "dr_branch_121_Yes",
    "dr_branch_12_Yes_unsat",
    "dr_branch_13_Yes_unsat",
    "dr_branch_14_Yes_unsat",
    "dr_branch_15_Yes_unsat",
    "dr_branch_16_Yes_unsat",
    "dr_branch_17_Yes_unsat",
    "dr_branch_18_Yes_unsat",
    "dr_branch_19_Yes_unsat",
    "dr_branch_110_Yes_unsat",
    "dr_branch_111_Yes_unsat",
    "dr_branch_112_Yes_unsat",
    "dr_branch_113_Yes_unsat",
    "dr_branch_114_Yes_unsat",
    "dr_branch_115_Yes_unsat",
    "dr_branch_116_Yes_unsat",
    "dr_branch_117_Yes_unsat",
    "dr_branch_118_Yes_unsat",
    "dr_branch_119_Yes_unsat",
    "dr_branch_120_Yes_unsat",
    "dr_branch_121_Yes_unsat"
]

bm_results = []
total_count = 0
failed_count = 0
failed_tests = []
passed_count = 0
crashed_count = 0
crashed_tests = []

ref_cwd = os.getcwd();
arch = platform.machine()
full_hostname = platform.node()
hostname=full_hostname

for config in configs:
    util.log_heading(config["NAME"], character="-")
    if do_clean:
        util.chdir(TD_ROOT)
        util.run_command("make clean", verbose=False)
        util.chdir(TBBROOT)
        util.run_command("make clean", verbose=False)
    if do_build:
        util.chdir(TD_ROOT)
        util.run_command("make", verbose=False)
        util.chdir(TBBROOT)
        util.run_command("make", verbose=False)
    util.chdir(ref_cwd)
    
    try:
        clean_string = config["CLEAN_LINE"]
        util.run_command(clean_string, verbose=False)
    except:
        print "Clean failed"
        
    build_string = config["BUILD_LINE"]
    util.run_command(build_string, verbose=False)

    for benchmark in benchmarks:
        total_count = total_count + 1
        try:
            util.log_heading(benchmark, character="=")
            if do_run:
                
                run_string = config["RUN_LINE"] + benchmark
                util.run_command(run_string, verbose=False)
                
                test_res = 0
                if "Yes" in benchmark and "fixed" not in benchmark and "unsat" not in benchmark:
                    test_res = 1
                
                f = open('violations.out', 'r')
                file_line = f.readline()
                f.close()
                output_res = 0
                if "Data race detected" in file_line:
                    output_res = 1
                    if args.d == "ptracer":
                        if "branch" in benchmark and config["NAME"] == "dr_detector":
                            solver_string = "python " + SOLVER_ROOT + "/solver.py -t " + ref_cwd + "/traces"
                            util.run_command(solver_string, verbose=False)
                            f = open('sat_check.out', 'r')
                            f_line = f.readline()
                            f.close()
                            if "unsat" in benchmark:
                                if "unsat" in f_line:
                                    output_res = 0
                                else:
                                    output_res = 1
                            else:
                                if "sat" in f_line:
                                    output_res = 1
                                else:
                                    output_res = 0
                    
                if test_res == output_res:
                    util.log_message("*** TEST PASSED ***")
                    passed_count = passed_count + 1
                else:
                    util.log_message("*** TEST FAILED ***")
                    failed_count = failed_count + 1
                    f_test = []
                    f_test.append(config["NAME"])
                    f_test.append(benchmark)
                    failed_tests.append(f_test)
                    if test_res == 1 and output_res == 0:
                        util.log_message("Test has a data race but was not reported by tool")
                    else:
                        util.log_message("Test has no data race but tool reported data race")

        except util.ExperimentError, e:
            print "Error: %s" % e
            print "-----------"
            print "%s" % e.output
            print "-----------"
            crashed_count = crashed_count + 1
            c_test = []
            c_test.append(config["NAME"])
            c_test.append(benchmark)
            crashed_tests.append(c_test)
            continue

print ""
print "********* TEST REPORT **********"
print "Total testcases run: " + str(total_count)
print "Total testcases PASSED: " + str(passed_count)
print "Total testcases FAILED: " + str(failed_count)
if failed_count != 0:
    print "FAILED testcases:"
    print failed_tests
    print "Total testcases CRASHED: " + str(crashed_count)
if crashed_count != 0:
    print "CRASHED testcases:"
    print crashed_tests
print "********* TEST REPORT ENDS **********"
