

print "hello"


def runPyProg(exp):
    print exp
    exec exp in globals()
    return