class CompOperEnum:
    eq = 1
    neq = 2
    gt = 3
    lt = 4
    gte = 5
    lte = 6

class Operation:
    def __init__(self):
        self.taskId = ""
        self.var = ""
        self.id = 0  

class RWOperation(Operation):
    def __init__(self):
        self.value = ""
        self.iswrite = False

class SpawnOperation(Operation):
    def __init__(self):
        self.chilId = ""
        self.lastOperId = 0

class LockPairOperation(Operation):
    def __init__(self):
        self.unlockId = 0

class BranchCheckOperation(Operation):
    def __init__(self):
        self.compOper = CompOperEnum.eq
        self.value = ""
