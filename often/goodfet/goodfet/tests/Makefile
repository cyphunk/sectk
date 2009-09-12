TARGETS=monitor cc msp430x2
GOODFET=/dev/ttyUSB0


all: $(TARGETS)
clean:
	rm -f $(TARGETS) *~

monitor: monitor.zsh
	GOODFET=/dev/ttyUSB0 ./monitor.zsh >monitor
cc: cc.zsh
	./cc.zsh >cc
msp430x2: msp430x2.zsh
	./msp430x2.zsh >msp430x2
