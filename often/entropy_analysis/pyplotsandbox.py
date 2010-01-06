from pylab import *
from matplotlib.ticker import MultipleLocator, FormatStrFormatter

majorLocator   = MultipleLocator(1)
majorFormatter = FormatStrFormatter('%d')
minorLocator   = MultipleLocator(.2)


t = arange(0.0, 10.0, 0.01)
s = sin(2*pi*t)*exp(-t*0.01)

ax = subplot(111)
plot(t,s)

ax.xaxis.set_major_locator(majorLocator)
ax.xaxis.set_major_formatter(majorFormatter)
#for the minor ticks, use no labels; default NullFormatter
ax.xaxis.set_minor_locator(minorLocator)

#ax.axis(pickradius=35)
#print ax.xaxis.get_pickradius()

#f = gcf()
#f.SubplotParams()
subplots_adjust(left=0)
show()
savefig("test")
