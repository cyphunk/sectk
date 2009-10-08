#!/bin/bash
# some common dtrace commands
# others:
# http://www.brendangregg.com/DTrace/dtrace_oneliners.txt

case "$1" in
  open)
	echo "open 		open* calls"
	sudo dtrace -n 'syscall::open*:entry { printf("%s %s",execname,copyinstr(arg0)); }'
	;;
  exec_sm)
	echo "exec_sm 	new processes with time and arguments"
	sudo dtrace -qn 'syscall::exec*:return { printf("%Y %s\n",walltimestamp,curpsinfo->pr_psargs); }'
	;;
  exec)
	echo "exec 		new processes with trace"
	sudo dtrace -n 'proc:::exec-success { trace(curpsinfo->pr_psargs); }'
	;;
  syscall_pid)
	echo "syscall_pid	syscall count by process"
	sudo dtrace -n 'syscall:::entry { @num[pid,execname] = count(); }'
	;;
  syscall_prog)
	echo "syscall_prog	syscall count by program"
	sudo dtrace -n 'syscall:::entry { @num[execname] = count(); }'
	;;
  signals)
	echo "signals		signals by pid"
	sudo dtrace -n 'proc:::signal-send /pid/ { printf("%s -%d %d",execname,args[2],args[1]->pr_pid); }'
	;;
  findother)
	echo "findother		find other local dtrace scripts"
	# http://bdgregg.blogspot.com/2008/02/dtracetoolkit-in-macos-x.html
	grep -l DTrace /usr/bin/*
	grep -l DTrace /bin/*
	;;
  *)
	echo "Usage "
	echo ""
	echo "findother		find other local dtrace scripts"
	echo "open 		open* calls"
	echo "exec_sm 	new processes with time and arguments"
	echo "exec 		new processes with trace"
	echo "syscall_pid	syscall count by process"
	echo "syscall_prog	syscall count by program"
	echo "signals		signals by pid"
	echo ""
	;;
esac
