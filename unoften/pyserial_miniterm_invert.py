#!/System/Library/Frameworks/Python.framework/Versions/2.5/Resources/Python.app/Contents/MacOS/Python

# Very simple serial terminal
# (C)2002-2009 Chris Liechti <cliechti@gmx.net>

# Input characters are sent directly (only LF -> CR/LF/CRLF translation is
# done), received characters are displayed as is (or escaped trough pythons
# repr, useful for debug purposes)


import sys, os, serial, threading

EXITCHARCTER = '\x1d'   # GS/CTRL+]
MENUCHARACTER = '\x14'  # Menu: CTRL+T


def key_description(character):
    """generate a readable description for a key"""
    ascii_code = ord(character)
    if ascii_code < 32:
        return 'Ctrl+%c' % (ord('@') + ascii_code)
    else:
        return repr(character)

# help text, starts with blank line! it's a function so that the current values
# for the shortcut keys is used and not the value at program start
def get_help_text():
    return """
--- pySerial - miniterm - help
---
--- %(exit)-8s Exit program
--- %(menu)-8s Menu escape key, followed by:
--- Menu keys:
---       %(itself)-8s Send the menu character itself to remote
---       %(exchar)-8s Send the exit character to remote
---       %(info)-8s Show info
---       %(upload)-8s Upload file (prompt will be shown)
--- Toggles:
---       %(rts)s  RTS          %(echo)s  local echo
---       %(dtr)s  DTR          %(break)s  BREAK
---       %(lfm)s  line feed    %(repr)s  Cycle repr mode
---
--- Port settings (%(menu)s followed by the following):
--- 7 8           set data bits
--- n e o s m     change parity (None, Even, Odd, Space, Mark)
--- 1 2 3         set stop bits (1, 2, 1.5)
--- b             change baud rate
--- x X           disable/enable software flow control
--- r R           disable/enable hardware flow control
""" % {
    'exit': key_description(EXITCHARCTER),
    'menu': key_description(MENUCHARACTER),
    'rts': key_description('\x12'),
    'repr': key_description('\x01'),
    'dtr': key_description('\x04'),
    'lfm': key_description('\x0c'),
    'break': key_description('\x02'),
    'echo': key_description('\x05'),
    'info': key_description('\x09'),
    'upload': key_description('\x15'),
    'itself': key_description(MENUCHARACTER),
    'exchar': key_description(EXITCHARCTER),
}

# first choose a platform dependant way to read single characters from the console
global console

if os.name == 'nt':
    import msvcrt
    class Console:
        def __init__(self):
            pass

        def setup(self):
            pass    # Do nothing for 'nt'

        def cleanup(self):
            pass    # Do nothing for 'nt'

        def getkey(self):
            while 1:
                z = msvcrt.getch()
                if z == '\0' or z == '\xe0':    #functions keys
                    msvcrt.getch()
                else:
                    if z == '\r':
                        return '\n'
                    return z

    console = Console()

elif os.name == 'posix':
    import termios, sys, os
    class Console:
        def __init__(self):
            self.fd = sys.stdin.fileno()

        def setup(self):
            self.old = termios.tcgetattr(self.fd)
            new = termios.tcgetattr(self.fd)
            new[3] = new[3] & ~termios.ICANON & ~termios.ECHO & ~termios.ISIG
            new[6][termios.VMIN] = 1
            new[6][termios.VTIME] = 0
            termios.tcsetattr(self.fd, termios.TCSANOW, new)
            #s = ''    # We'll save the characters typed and add them to the pool.

        def getkey(self):
            c = os.read(self.fd, 1)
            return c

        def cleanup(self):
            termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.old)

    console = Console()

    def cleanup_console():
        console.cleanup()

    console.setup()
    sys.exitfunc = cleanup_console      #terminal modes have to be restored on exit...

else:
    raise "Sorry no implementation for your platform (%s) available." % sys.platform


CONVERT_CRLF = 2
CONVERT_CR   = 1
CONVERT_LF   = 0
NEWLINE_CONVERISON_MAP = ('\n', '\r', '\r\n')
LF_MODES = ('LF', 'CR', 'CR/LF')

REPR_MODES = ('raw', 'some control', 'all control', 'hex', 'inverted')

class Miniterm:
    def __init__(self, port, baudrate, parity, rtscts, xonxoff, echo=False, convert_outgoing=CONVERT_CRLF, repr_mode=0, bytesize=serial.EIGHTBITS, encode=False):
        self.serial = serial.Serial(port, baudrate, parity=parity, rtscts=rtscts, xonxoff=xonxoff, timeout=0.7, bytesize=bytesize)
        self.echo = echo
        self.repr_mode = repr_mode
        self.convert_outgoing = convert_outgoing
        self.newline = NEWLINE_CONVERISON_MAP[self.convert_outgoing]
        self.dtr_state = True
        self.rts_state = True
        self.break_state = False
	self.encode = encode

    def start(self):
        self.alive = True
        # start serial->console thread
        self.receiver_thread = threading.Thread(target=self.reader)
        self.receiver_thread.setDaemon(1)
        self.receiver_thread.start()
        # enter console->serial loop
        self.transmitter_thread = threading.Thread(target=self.writer)
        self.transmitter_thread.setDaemon(1)
        self.transmitter_thread.start()

    def stop(self):
        self.alive = False

    def join(self, transmit_only=False):
        self.transmitter_thread.join()
        if not transmit_only:
            self.receiver_thread.join()

    def dump_port_settings(self):
        sys.stderr.write("\n--- Settings: %s  %s,%s,%s,%s\n" % (
            self.serial.portstr,
            self.serial.baudrate,
            self.serial.bytesize,
            self.serial.parity,
            self.serial.stopbits,
        ))
        sys.stderr.write('--- RTS %s\n' % (self.rts_state and 'active' or 'inactive'))
        sys.stderr.write('--- DTR %s\n' % (self.dtr_state and 'active' or 'inactive'))
        sys.stderr.write('--- BREAK %s\n' % (self.break_state and 'active' or 'inactive'))
        sys.stderr.write('--- software flow control %s\n' % (self.serial.xonxoff and 'active' or 'inactive'))
        sys.stderr.write('--- hardware flow control %s\n' % (self.serial.rtscts and 'active' or 'inactive'))
        sys.stderr.write('--- data escaping: %s\n' % (REPR_MODES[self.repr_mode],))
        sys.stderr.write('--- linefeed: %s\n' % (LF_MODES[self.convert_outgoing],))

    def reader(self):
        """loop and copy serial->console"""
        while self.alive:
            data = self.serial.read(1)

            if self.repr_mode == 0:
                # direct output, just have to care about newline setting
                if data == '\r' and self.convert_outgoing == CONVERT_CR:
                    sys.stdout.write('\n')
                else:
                    sys.stdout.write(data)
            elif self.repr_mode == 1:
                # escape non-printable, let pass newlines
                if self.convert_outgoing == CONVERT_CRLF and data in '\r\n':
                    if data == '\n':
                        sys.stdout.write('\n')
                    elif data == '\r':
                        pass
                elif data == '\n' and self.convert_outgoing == CONVERT_LF:
                    sys.stdout.write('\n')
                elif data == '\r' and self.convert_outgoing == CONVERT_CR:
                    sys.stdout.write('\n')
                else:
                    sys.stdout.write(repr(data)[1:-1])
            elif self.repr_mode == 2:
                # escape all non-printable, including newline
                sys.stdout.write(repr(data)[1:-1])
            elif self.repr_mode == 3:
                # escape everything (hexdump)
                for character in data:
                    sys.stdout.write("%s " % character.encode('hex'))
            elif self.repr_mode == 4:
                for character in data:
		    sys.stdout.write(self.decode(character))
            elif self.repr_mode == 5:
                for character in data:
		    if character < 129:
	                    sys.stdout.write("%s " % character.encode('rot_13'))
		    else:
                    	    sys.stdout.write("%s " % character.encode('hex'))

		
            sys.stdout.flush()


    def decodeOLD(self, char):
	# make char into int
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	#print num
	carrybit = num & 0x01 # carry 00000001
	# put the carry bit at 10000000
	carrybit = carrybit << 7
	num = num >>1
	num = num + carrybit
	#num = ~num & 0x7F
	num = ~(num & 0x7F) & 0x7f
	#print num
	return chr(num)	

    def decodeI(self, char):
	# No shift, just invert. 
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	#print hex(num)
	#num = ~num & 0x7F
	#num = num >> 1
	num = ~num & 0xFF
	#num = ~(num & 0x7F) & 0x7f
	#print hex(num)
	return chr(num)	

    def decodeS1I(self, char):
	# Shift right 1 (remove lsb) invert
	"""Doesnt work very well but does get must chars.  locks up.  but does show:
	   Cannot open *bootrom.bin".
	   Press any key to stop auto-boot..."""
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	num = num >> 1
	num = ~num & 0xFF
	return chr(num)	

    def rotateR(self, int, N): # rotate a SINGLE BYTE right N steps.  Shift right, with carry
	carymask = (0x1<<N)-1 # N=3: 00001000 - 1 = 00000111 
	cary = int & carymask # N=3: int 00110011, cary = 011
	cary = cary<<(8-N)    # N=3: cary = 01100000
	int = int>>N          # N=3: int 00110011 = ___00110
	int = int + cary
	return int
    def rotateL(self, int, N): # rotate a SINGLE BYTE left N steps.  Shift left, with carry
	carymask = 0xFF>>(8-N)<<(8-N) # its ugly but, err.  yeah no excuse. Creates 11100000 for N=3
	cary = int & carymask # N=3: int 00110011, cary = 00100000
	cary = cary>>(8-N)    # N=3: cary = 00000001
	int = int<<N          # N=3: int 00110011 = 00110011___
	int = int + cary      # 00110011000 + 00000001 = 00110011001
	int = int & 0xFF      # 00110011000 = 10011001
	return int #10011001
    def invert(self, int): # BYTE only
	return ~int & 0xFF

    def decodeS1I(self, char): # Doesnt work very well but does get must chars.  locks up
	# Shift right 1 (remove lsb) invert
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	num = num >> 1
	num = ~num & 0x7F
	return chr(num)	

    def decodeS1R2I(self, char): # Gets something 
	# Shift right 1 (remove lsb) rotate right 2.  invert
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	num = num >> 1

	carry = num & 0x03 # carry 00000011, put the carry bit at 11000000
	carry = carry << 6

	num = num + carry

	num = ~num & 0x7F
	return chr(num)	

    def encodeOLD(self, char):
	# make char into int
	num=int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	#print num
	carrybit = num & 0x10 # carry 10000000
	# put the carry bit at 00000001
	carrybit = carrybit >> 7
	num = num << 1
	num = num & carrybit
	num = ~num & 0x7F
	#print num
	return chr(num)	

    def decode(self, char):
	# make char into int
	num = int(char.encode('hex'), 16) # really? amazing how ugly simple casting can be
	num = self.invert(num)
	#num = self.rotateR(num, 1) # output okay but not best
	num = num>>1                # better output. proof 7bit ascii, LSB not needed
	#print num
	return chr(num)	

    def encode(self, char):
	num=int(char.encode('hex'), 16)
	num=self.invert(num)
	num = (num<<1)>>1
	num=self.rotateR(num, 1)
	""" with i pressed and varrying rotation
	6 = J
	5 = Z
	3 = k
	2 = K"""
	return chr(num)

    def writer(self):
        """loop and copy console->serial until EXITCHARCTER character is
           found. when MENUCHARACTER is found, interpret the next key
           locally.
        """
        menu_active = False
        try:
            while self.alive:
                try:
                    c = console.getkey()
                except KeyboardInterrupt:
                    c = '\x03'
                if menu_active:
                    if c == MENUCHARACTER or c == EXITCHARCTER: # Menu character again/exit char -> send itself
                        self.serial.write(c)                    # send character
                        if self.echo:
                            sys.stdout.write(c)
                    elif c == '\x15':                       # CTRL+U -> upload file
                        sys.stderr.write('\n--- File to upload: ')
                        sys.stderr.flush()
                        console.cleanup()
                        filename = sys.stdin.readline().rstrip('\r\n')
                        if filename:
                            try:
                                file = open(filename, 'r')
                                sys.stderr.write('--- Sending file %s ---\n' % filename)
                                while True:
                                    line = file.readline().rstrip('\r\n')
                                    if not line:
                                        break
                                    self.serial.write(line)
                                    self.serial.write('\r\n')
                                    # Wait for output buffer to drain.
                                    self.serial.flush()
                                    sys.stderr.write('.')   # Progress indicator.
                                sys.stderr.write('\n--- File %s sent ---\n' % filename)
                            except IOError, e:
                                sys.stderr.write('--- ERROR opening file %s: %s ---\n' % (filename, e))
                        console.setup()
                    elif c in '\x08hH?':                    # CTRL+H, h, H, ? -> Show help
                        sys.stderr.write(get_help_text())
                    elif c == '\x12':                       # CTRL+R -> Toggle RTS
                        self.rts_state = not self.rts_state
                        self.serial.setRTS(self.rts_state)
                        sys.stderr.write('--- RTS %s ---\n' % (self.rts_state and 'active' or 'inactive'))
                    elif c == '\x04':                       # CTRL+D -> Toggle DTR
                        self.dtr_state = not self.dtr_state
                        self.serial.setDTR(self.dtr_state)
                        sys.stderr.write('--- DTR %s ---\n' % (self.dtr_state and 'active' or 'inactive'))
                    elif c == '\x02':                       # CTRL+B -> toggle BREAK condition
                        self.break_state = not self.break_state
                        self.serial.setBreak(self.break_state)
                        sys.stderr.write('--- BREAK %s ---\n' % (self.break_state and 'active' or 'inactive'))
                    elif c == '\x05':                       # CTRL+E -> toggle local echo
                        self.echo = not self.echo
                        sys.stderr.write('--- local echo %s ---\n' % (self.echo and 'active' or 'inactive'))
                    elif c == '\x09':                       # CTRL+I -> info
                        self.dump_port_settings()
                    elif c == '\x01':                       # CTRL+A -> cycle escape mode
                        self.repr_mode += 1
                        if self.repr_mode > 3:
                            self.repr_mode = 0
                        sys.stderr.write('--- escape data: %s ---\n' % (
                            REPR_MODES[self.repr_mode],
                        ))
                    elif c == '\x0c':                       # CTRL+L -> cycle linefeed mode
                        self.convert_outgoing += 1
                        if self.convert_outgoing > 2:
                            self.convert_outgoing = 0
                        self.newline = NEWLINE_CONVERISON_MAP[self.convert_outgoing]
                        sys.stderr.write('--- line feed %s ---\n' % (
                            LF_MODES[self.convert_outgoing],
                        ))
                    #~ elif c in 'pP':                         # P -> change port XXX reader thread would exit
                    elif c in 'bB':                         # B -> change baudrate
                        sys.stderr.write('\n--- Baudrate: ')
                        sys.stderr.flush()
                        console.cleanup()
                        backup = self.serial.baudrate
                        try:
                            self.serial.baudrate = int(sys.stdin.readline().strip())
                        except ValueError, e:
                            sys.stderr.write('--- ERROR setting baudrate: %s ---\n' % (e,))
                            self.serial.baudrate = backup
                        else:
                            self.dump_port_settings()
                        console.setup()
                    elif c == '8':                          # 8 -> change to 8 bits
                        self.serial.bytesize = serial.EIGHTBITS
                        self.dump_port_settings()
                    elif c == '7':                          # 7 -> change to 8 bits
                        self.serial.bytesize = serial.SEVENBITS
                        self.dump_port_settings()
                    elif c in 'eE':                         # E -> change to even parity
                        self.serial.parity = serial.PARITY_EVEN
                        self.dump_port_settings()
                    elif c in 'oO':                         # O -> change to odd parity
                        self.serial.parity = serial.PARITY_ODD
                        self.dump_port_settings()
                    elif c in 'mM':                         # M -> change to mark parity
                        self.serial.parity = serial.PARITY_MARK
                        self.dump_port_settings()
                    elif c in 'sS':                         # S -> change to space parity
                        self.serial.parity = serial.PARITY_SPACE
                        self.dump_port_settings()
                    elif c in 'nN':                         # N -> change to no parity
                        self.serial.parity = serial.PARITY_NONE
                        self.dump_port_settings()
                    elif c == '1':                          # 1 -> change to 1 stop bits
                        self.serial.stopbits = serial.STOPBITS_ONE
                        self.dump_port_settings()
                    elif c == '2':                          # 2 -> change to 2 stop bits
                        self.serial.stopbits = serial.STOPBITS_TWO
                        self.dump_port_settings()
                    elif c == '3':                          # 3 -> change to 1.5 stop bits
                        self.serial.stopbits = serial.STOPBITS_ONE_POINT_FIVE
                        self.dump_port_settings()
                    elif c in 'xX':                         # X -> change software flow control
                        self.serial.xonxoff = (c == 'X')
                        self.dump_port_settings()
                    elif c in 'rR':                         # R -> change hardware flow control
                        self.serial.rtscts = (c == 'R')
                        self.dump_port_settings()
                    else:
                        sys.stderr.write('--- unknown menu character %s --\n' % key_description(c))
                    menu_active = False
                elif c == MENUCHARACTER: # next char will be for menu
                    menu_active = True
                elif c == EXITCHARCTER: 
                    self.stop()
                    break                                   # exit app
                elif c == '\n':
                    self.serial.write(self.newline)         # send newline character(s)
                    if self.echo:
                        sys.stdout.write(c)                 # local echo is a real newline in any case
                        sys.stdout.flush()
                else:
		    if self.encode:
	                    self.serial.write(self.encode(c))                    # send character
		    else:
			    self.serial.write(c)
                    #if self.echo:
                    #    sys.stdout.write(c)
                    #    sys.stdout.flush()
        except:
            self.alive = False
            raise

def main():
    import optparse

    parser = optparse.OptionParser(
        usage = "%prog [options] [port [baudrate]]",
        description = "Miniterm - A simple terminal program for the serial port."
    )

    parser.add_option("-p", "--port",
        dest = "port",
        help = "port, a number (default 0) or a device name (deprecated option)",
        default = None
    )

    parser.add_option("-b", "--baud",
        dest = "baudrate",
        action = "store",
        type = 'int',
        help = "set baud rate, default %default",
        default = 9600
    )

    parser.add_option("--parity",
        dest = "parity",
        action = "store",
        help = "set parity, one of [N, E, O, S, M], default=N",
        default = 'N'
    )

    parser.add_option("-e", "--echo",
        dest = "echo",
        action = "store_true",
        help = "enable local echo (default off)",
        default = False
    )

    parser.add_option("--rtscts",
        dest = "rtscts",
        action = "store_true",
        help = "enable RTS/CTS flow control (default off)",
        default = False
    )

    parser.add_option("--xonxoff",
        dest = "xonxoff",
        action = "store_true",
        help = "enable software flow control (default off)",
        default = False
    )

    parser.add_option("--cr",
        dest = "cr",
        action = "store_true",
        help = "do not send CR+LF, send CR only",
        default = False
    )

    parser.add_option("--lf",
        dest = "lf",
        action = "store_true",
        help = "do not send CR+LF, send LF only",
        default = False
    )

    parser.add_option("-D", "--debug",
        dest = "repr_mode",
        action = "count",
        help = """debug received data (escape non-printable chars)
--debug can be given multiple times:
0: just print what is received
1: escape non-printable characters, do newlines as unusual
2: escape non-printable characters, newlines too
3: hex dump everything
4: invert input (run decode)""",
        default = 0
    )

    parser.add_option("--rts",
        dest = "rts_state",
        action = "store",
        type = 'int',
        help = "set initial RTS line state (possible values: 0, 1)",
        default = None
    )

    parser.add_option("--dtr",
        dest = "dtr_state",
        action = "store",
        type = 'int',
        help = "set initial DTR line state (possible values: 0, 1)",
        default = None
    )

    parser.add_option("-q", "--quiet",
        dest = "quiet",
        action = "store_true",
        help = "suppress non error messages",
        default = False
    )

    parser.add_option("--exit-char",
        dest = "exit_char",
        action = "store",
        type = 'int',
        help = "ASCII code of special character that is used to exit the application",
        default = 0x1d
    )

    parser.add_option("--menu-char",
        dest = "menu_char",
        action = "store",
        type = 'int',
        help = "ASCII code of special character that is used to control miniterm (menu)",
        default = 0x14
    )
    parser.add_option("--decode",
        dest = "decode",
        action = "store_true",
        help = "set up for playing with inverted serial: decode input",
        default = False
    )
    parser.add_option("--encode",
        dest = "encode",
        action = "store_true",
        help = "set up for playing with inverted serial: encode output (well this wont work offcourse for stopbit, etc)",
        default = False
    )

    parser.add_option("--bytesize",
        dest = "bytesize",
        action = "store",
	type = 'int',
        help = "set up bytesize of 8 (default), 7, 6 or 5",
        default = serial.EIGHTBITS
    )

    (options, args) = parser.parse_args()

    options.parity = options.parity.upper()
    if options.parity not in 'NEOSM':
        parser.error("invalid parity")

    if options.cr and options.lf:
        parser.error("only one of --cr or --lf can be specified")

    if options.dtr_state is not None and options.rts_state is not None and options.dtr_state == options.rts_state:
        parser.error('--exit-char can not be the same as --menu-char')

    global EXITCHARCTER, MENUCHARACTER
    EXITCHARCTER = chr(options.exit_char)
    MENUCHARACTER = chr(options.menu_char)

    port = options.port
    baudrate = options.baudrate
    if args:
        if options.port is not None:
            parser.error("no arguments are allowed, options only when --port is given")
        port = args.pop(0)
        if args:
            try:
                baudrate = int(args[0])
            except ValueError:
                parser.error("baud rate must be a number, not %r" % args[0])
            args.pop(0)
        if args:
            parser.error("too many arguments")
    else:
        if port is None: port = 0

    convert_outgoing = CONVERT_CRLF
    if options.cr:
        convert_outgoing = CONVERT_CR
    elif options.lf:
        convert_outgoing = CONVERT_LF

    if options.decode:
	options.repr_mode=4
	#options.bytesize=serial.SEVENBITS
	#options.bytesize=serial.EIGHTBITS
	#options.parity='N'
	# OSX/Unix termios.h api does not openly support MARK and SPACE parity.
	# This work around provided by:
	# http://mail.python.org/pipermail/python-list/2005-November/351210.html
	# http://www.lothosoft.ch/thomas/libmip/markspaceparity.php
	# We have to set this after the serial/Miniterm object is created.  
	# ALSO, there is no CMSPAR constant on OSX, so we define this by guessing
	# from: 
	# http://forums.purebasic.com/english/viewtopic.php?t=35363&sid=27dfb104e3f84013e20911d6b16e1852

	

    try:
        miniterm = Miniterm(
            port,
            baudrate,
            options.parity,
            rtscts=options.rtscts,
            xonxoff=options.xonxoff,
            echo=options.echo,
            convert_outgoing=convert_outgoing,
            repr_mode=options.repr_mode,
	    bytesize=options.bytesize,
	    encode=options.encode,
        )
	if options.decode and False:
	    print "setting up"
	    # See options.decode definition code for explination:
	    import termios
	    if (sys.hexversion < 0x020100f0):
    		import TERMIOS
	    else:
    		TERMIOS = termios
	    CMSPAR = 0x40000000
	    iflag, oflag, cflag, lflag, ispeed, ospeed, cc = termios.tcgetattr(miniterm.serial)
	    cflag |= TERMIOS.PARENB | CMSPAR # To select SPACE parity
	    cflag &= ~ TERMIOS.PARODD
	    cflag |= TERMIOS.PARENB | CMSPAR | TERMIOS.PARODD # to select MARK parity
	    termios.tcsetattr(miniterm.serial, termios.TCSANOW, [iflag, oflag, cflag, lflag, ispeed, ospeed, cc])

    except serial.SerialException:
        sys.stderr.write("could not open port %r\n" % port)
        sys.exit(1)

    if not options.quiet:
        sys.stderr.write('--- Miniterm on %s: %d,%s,%s,%s ---\n' % (
            miniterm.serial.portstr,
            miniterm.serial.baudrate,
            miniterm.serial.bytesize,
            miniterm.serial.parity,
            miniterm.serial.stopbits,
        ))
        sys.stderr.write('--- Quit: %s  |  Menu: %s | Help: %s followed by %s ---\n' % (
            key_description(EXITCHARCTER),
            key_description(MENUCHARACTER),
            key_description(MENUCHARACTER),
            key_description('\x08'),
        ))

    if options.dtr_state is not None:
        if not options.quiet:
            sys.stderr.write('--- forcing DTR %s\n' % (options.dtr_state and 'active' or 'inactive'))
        miniterm.serial.setDTR(options.dtr_state)
        miniterm.dtr_state = options.dtr_state
    if options.rts_state is not None:
        if not options.quiet:
            sys.stderr.write('--- forcing RTS %s\n' % (options.rts_state and 'active' or 'inactive'))
        miniterm.serial.setRTS(options.rts_state)
        miniterm.rts_state = options.rts_state


    miniterm.start()
    miniterm.join(True)
    if not options.quiet:
        sys.stderr.write("\n--- exit ---\n")
    miniterm.join()


if __name__ == '__main__':
    main()
