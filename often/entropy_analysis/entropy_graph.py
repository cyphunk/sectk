""" Entropy scan
    H() and entropy_scan() originally by Ero Carrera (blog.dkbza.org)
    
    Modified May 2007 by cyphunk (deadhacker.com)
    Modified Dec 2009 by cyphunk

    USAGE:
    cmd [target_path]
    """

# FLAGS:
SHOWPROGRESS = 1       # Show console progress bar?
PRINTONTHRESHOLD = 0   # When block is > than threshold
                       # print first 16 bytes in both
                       # hex and ascii.  Set to 0 to turn
                       # off.
ONLYFIRSTBLOCK = 0     # Set to 1 it will only print the first
                       # block that goes over threshold and not
                       # blocks > threshold that are only offset
                       # by 1.  By setting to zero block windows
                       # that match will be printed.
BLOCKSIZE = 256        # size of blocks scanned.

import math
import random # to build test data
from pylab import *
from matplotlib.ticker import MultipleLocator, FormatStrFormatter
from progressBar import *
from binascii import hexlify
import string
import os
import cPickle # cache results

def H(data):
  if not data:
    return 0
  entropy = 0
  for x in range(256):
    p_x = float(data.count(chr(x)))/len(data)
    if p_x > 0:
      entropy += - p_x*math.log(p_x, 2)
  return entropy

def entropy_scan (data, block_size) :
  if SHOWPROGRESS:
      progress = progressBar(0, len(data) - block_size, 77)
  # creates blocks of block_size for all possible offsets ('x'):
  blocks = (data[x : block_size + x] for x in range (len (data) - block_size))
  i = 0
  for block in (blocks) :
    i += 1
    if SHOWPROGRESS:
        progress(i)
    yield H (block)

# performance improvement if you have psyco
try:
    import psyco
    psyco.full()
    print "got psyco"
except ImportError:
    pass

# get target file as argument var or via GUI dialog:
filename = ""
if sys.argv[1:]:
    filename = sys.argv[1]
else:
    import tkFileDialog
    from Tkinter import *
    root = Tk()
    root.withdraw()
    filename = tkFileDialog.askopenfilename(title="Target binary",
                                        filetypes=[("All files", "*")])

# run, print graph:

if filename:
    # Open file and scan for entropy:
    if os.path.splitext(filename)[1] == ".entropy":
        print "Reading cached '.entropy'"
        results = cPickle.load(open(filename, 'rb'))
        filename = os.path.splitext(filename)[0]
        raw = open(filename, 'rb').read()
    else:      
        raw = open(filename, 'rb').read()
        # debug:
        """
        import randosam
        raw = ''.join (
        [chr (random.randint (0, 64)) for x in xrange (1024)] +
        [chr (random.randint (0, 255)) for x in xrange (1024)] +
        [chr (random.randint (0, 64)) for x in xrange (1024)] )
        """
        results = list( entropy_scan(raw,BLOCKSIZE) )
        cPickle.dump(results, open(filename+".entropy", 'wb')) 

    # Print blocks that are above a defined threshold of entropy:
    if PRINTONTHRESHOLD > 0:
        print
        found = 0
        for i in range(len(results)):
            if results[i] > PRINTONTHRESHOLD:
                if found == 0:
                    table = string.maketrans("rnt", '   ') # don't like newlines
                    #blockstr = string.translate(str(raw[i : i+16]), table) # translate to string value
                    print "0x%08X %.2f: %s %s" % (i, results[i], hexlify(raw[i : i+8]),
                                                     hexlify(raw[i+8 : i+16]))
                    #%.3f - %016X / %s" % (i, results[i], raw[i : i + 16], raw[i : i + 16])
                    found = ONLYFIRSTBLOCK
            else:
                found = 0

    # Plot
    filesize = os.path.getsize(filename)
    imgdpi = 100
    imgwidth = filesize / imgdpi
    if imgwidth > 327:
        imgwidth = 327
      
    majorLocator   = MultipleLocator(0x400)   # note every 1024 bytes
    majorFormatter = FormatStrFormatter('%X') # %d = for decimal byte notation

    ax = subplot(111)
    plot(results, linewidth=0.1, antialiased=False)
    subplots_adjust(left=0.02, right=0.99, bottom=0.2)

    ax.axis([0,filesize,0,8])
    ax.xaxis.set_major_locator(majorLocator)
    ax.xaxis.set_major_formatter(majorFormatter)
    xticks(rotation=315)
    
    xlabel('block (byte offset)')
    ylabel('entropy')
    title('Entropy levels')

    grid(True)
    
    img = gcf()
    img.set_size_inches(imgwidth, 6)
    img.savefig(filename+".png", dpi=imgdpi)

    draw()
    show()
