import matplotlib.pyplot as plt
from src.Image import Image 
from src.loadImage import loadImage

def showImage():

    image = loadImage(246, 1)

    imgplot = plt.imshow(image.data)

    plt.show()
