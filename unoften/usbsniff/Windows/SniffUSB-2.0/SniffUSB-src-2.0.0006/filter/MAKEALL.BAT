rm -v *.log *.err *.wrn
rm -rv obj*
call makewxpchk.bat
call makewxpfre.bat
call makewnetamd64chk.bat
call makewnetamd64fre.bat

