class Logger:
    def __init__(self, verbose=False):
        self.verbose = verbose

    def log(self, msg: str):
        print(msg)

    def verbose_log(self, msg: str):
        if self.verbose:
            print(msg)


logger = Logger()