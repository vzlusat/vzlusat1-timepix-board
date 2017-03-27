class Image:

    def __init__(self, id):

        self.id = id

        format = 0
        mode = 0

        thr = 0
        bias = 0
        exposure = 0
        filtering = 0
        filteredPixels = 0
        originalPixels = 0
        minOriginal = 0
        maxOriginal = 0
        minFiltered = 0
        maxFiltered = 0
        temperature = 0
        tempLimit = 0
        pxllimit = 0
        uav1thr = 0
        chunkid = 0
        attitude = []
        position = []

        data = []
