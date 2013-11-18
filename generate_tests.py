# CMPUT 481 Assignment 2
# Aaron Krebs <akrebs@ualberta.ca>


def generate_tests():
    test_sizes = [1000000,5000000,10000000,50000000]
    process_pool_sizes = [1,2,4,8,16]
    repeats = 7
    lineCount = 2 # skip shell header line

    print "#!/bin/sh"

    # sequential runs
    for i in test_sizes:
        for r in xrange(0, repeats):
            print "./{exe} -n {n} >> output{lineCount}.txt".format(
                exe='qsort', n=i, lineCount=lineCount)
            lineCount += 1

    # psrs runs
    for i in test_sizes:
        for np in process_pool_sizes:
            for r in xrange(0, repeats):
                print "mpiexec -np {np} ./{exe} -n {n} >> output{lineCount}.txt"\
                    .format(np=np, exe='psrs', n=i, lineCount=lineCount)
                lineCount += 1

if __name__ == "__main__":
	generate_tests()

