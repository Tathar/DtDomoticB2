import sys
import pathlib
import codecs
from importlib import reload

sys.stdin.reconfigure(encoding='iso-8859-1')
numfile = 0
numfile += 1

filename = pathlib.Path("./debug{0}.log".format(numfile))
print(filename)
while filename.exists():
    numfile += 1
    filename = pathlib.Path("./debug{0}.log".format(numfile))
    print(filename)



file = filename.open(mode="w", encoding="ISO-8859-1")

for line in sys.stdin:
    if  "0;2497;setup" in line:
        print('new file')
        numfile += 1
        filename = pathlib.Path("./debug{0}.log".format(numfile))
        while filename.exists():
            numfile += 1
            filename = pathlib.Path("./debug{0}.log".format(numfile))
        file = filename.open(mode="w", encoding="ISO-8859-1")
    if file.writable():
        file.write(line)

    # print(line,end='')
