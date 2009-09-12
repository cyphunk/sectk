compile: 
	nanoc compile
upload: compile
	#--delete removed old releases
	rsync --exclude .svn -ave ssh output/* travisutk,goodfet@web.sourceforge.net:htdocs/
auto:
	nanoc autocompile
