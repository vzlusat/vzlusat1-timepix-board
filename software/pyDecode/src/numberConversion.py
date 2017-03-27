def bytesToInt16(byte1, byte2):

    return byte2<<8 | byte1

def bytesToInt32(byte1, byte2, byte3, byte4):

    return byte4<<24 | byte3<<16 | byte2<<8 | byte1

def bytesToFloat(byte1, byte2, byte3, byte4):

    return bytesToInt32(byte1, byte2, byte3, byte4)
