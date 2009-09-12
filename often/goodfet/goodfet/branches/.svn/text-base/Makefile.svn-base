
releases:
	zsh build.zsh
clean:
	rm -f *.zip
upload: releases
	rsync -ave ssh goodfet??.zip travisutk,goodfet@web.sourceforge.net:htdocs/releases/
