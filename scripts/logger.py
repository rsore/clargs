import datetime

class Logger:
    def __init__(self, verbose=False):
        self.verbose = verbose

    def log(self, msg: str):
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] {msg}")

    def verbose_log(self, msg: str):
        if self.verbose:
            self.log(msg)


logger = Logger()