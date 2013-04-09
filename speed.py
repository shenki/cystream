from fx2load import *
from datetime import datetime

def time(n):
    buf='\x00'*n
    n=datetime.now()
    r=f.ep_bulk(buf,0x82,10000)
    n2=datetime.now()
    if not r: raise
    return n2-n


f.open(0x04b4,0x1003)

for n in range(20):
    b=(n+1) * 1024 * 1024 
    t=time(b)
    mb = float(b) / (1024*1024)
    mbps = mb / (t.seconds + (float(t.microseconds) / 10**6))
    print mb , 'MB', mbps , "MBps", str(t)  
