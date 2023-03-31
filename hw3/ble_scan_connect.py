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
for dev in devices:
    print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rss
si))
    addr.append(dev.addr)
    n += 1
    for (adtype, desc, value) in dev.getScanData():
        print (" %s = %s" % (desc, value))
number = input('Enter your device number: ')
print ('Device', number)
num = int(number)
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
    uuid = 0x1810
    testService = dev.getServiceByUUID(uuid)
    print("characteristics:", testService.getCharacteristics(0x2a36))
    ec = testService.getCharacteristics(0x2a36)[0]
    print("handle:", ec.valHandle)
    print("ec:", ec)
    dev.writeCharacteristic(ec.valHandle+1, b"\x01\x00")
    time.sleep(1.0)

    while True:
        if dev.waitForNotifications(100.0):
            print("get notification!")
            break
#
finally:
    dev.disconnect()
