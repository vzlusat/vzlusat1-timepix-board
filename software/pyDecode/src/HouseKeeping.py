import numpy

class HouseKeeping:

    def __init__(self):

        self.boot_count = 0
        self.images_taken = 0
        self.temperature = 0
        self.fram_status = 0
        self.medipix_status = 0
        self.time_since_boot = 0
        self.TIR_max = 0
        self.TIR_min = 0
        self.IR_max = 0
        self.IR_min = 0
        self.UV1_max = 0
        self.UV1_min = 0
        self.UV2_max = 0
        self.UV2_min = 0
        self.temp_max = 0
        self.temp_min = 0

    housekeeping_labels=["Boot count:",
                         "Images taken:",
                         "Temperature:",
                         "FRAM status:",
                         "Timepix status:",
                         "Time since boot:",
                         "TIR max:",
                         "TIR min:",
                         "IR max:",
                         "IR min:",
                         "UV1 max:",
                         "UV1 min:",
                         "UV2 max:",
                         "UV2 min:",
                         "Temperature max:",
                         "Temperature min:",
                         ]
