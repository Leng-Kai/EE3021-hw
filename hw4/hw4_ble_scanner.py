import time
from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print ("Discovered device", dev.addr)
        elif isNewData:
            print ("Received new data from", dev.addr)
scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n=0
addr = []
my_device_num = 0
for dev in devices:
    if dev.addr == "e3:9f:46:c7:39:e1":
        my_device_num = n
    #print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rrssi))
    addr.append(dev.addr)
    n += 1
    #for (adtype, desc, value) in dev.getScanData():
    #    print (" %s = %s" % (desc, value))
#number = input('Enter your device number: ')
#print ('Device', number)
#num = int(number)
print("my_device_num:", my_device_num)
num = my_device_num
print (addr[num])
#
print ("Connecting...")
dev = Peripheral(addr[num], 'random')
#
print ("Services...")
for svc in dev.services:
    print (str(svc))
#
try:
    des = dev.getDescriptors()
    testService = dev.getServiceByUUID(0x180d)
    heartrate = testService.getCharacteristics(0x2a37)[0]
    dev.writeCharacteristic(heartrate.valHandle+1, b"\x01\x00")

    msX = dev.getServiceByUUID(0x1807)
    magnetoX = msX.getCharacteristics(0x2a4c)[0]
    dev.writeCharacteristic(magnetoX.valHandle+1, b"\x01\x00")

    msY = dev.getServiceByUUID(0x1802)
    magnetoY = msY.getCharacteristics(0x2a4a)[0]
    dev.writeCharacteristic(magnetoY.valHandle+1, b"\x01\x00")

    msZ = dev.getServiceByUUID(0x1803)
    magnetoZ = msZ.getCharacteristics(0x2a6f)[0]
    dev.writeCharacteristic(magnetoZ.valHandle+1, b"\x01\x00")

    time.sleep(1.0)

    while True:
        if dev.waitForNotifications(100.0):
            print("\nheartrate:", int.from_bytes(heartrate.read(), byteorder='big'))
            val = int.from_bytes(magnetoX.read(), byteorder='big') & 0xffff
            if val >= (1 << 15):
                val = -((1 << 16) - val)
            print("magneto X:", val)
            val = int.from_bytes(magnetoY.read(), byteorder='big') & 0xffffff
            val = val >> 8
            if val >= (1 << 15):
                val = -((1 << 16) - val)
            print("magneto Y:", val)
            val = int.from_bytes(magnetoZ.read(), byteorder='big') & 0xffff
            if val >= (1 << 15):
                val = -((1 << 16) - val)
            print("magneto Z:", val)
#
finally:
    dev.disconnect()
