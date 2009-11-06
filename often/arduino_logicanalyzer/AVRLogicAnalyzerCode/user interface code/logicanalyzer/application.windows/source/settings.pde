int numOfChans = 6; // number of channels

int chanHeight = 20; // the height of each channel in pixels

int chanPadding = chanHeight / 2; // spacing between channels

int scrollBarHeight = chanHeight; // the height of the scroll bar

int bitWidth = chanHeight / 2; // width of a single bit

int bitsInView = 100; // how many bits in view at any time

int sideBarWidth = 20; // the side bar width

int topPadding = chanHeight; // spacing at the top of the screen

int bottomPadding = topPadding; // spacing at the bottom of the screen

int viewWidth = (bitsInView * bitWidth) + sideBarWidth; // final width of window
// do not modify

int viewHeight = (chanHeight * numOfChans) + (chanPadding * numOfChans) + topPadding + scrollBarHeight + bottomPadding; // final height of window
// do not modify

int scrollerWidth = 10; // the size of the green scroller on the scroll bar

int maxStore = viewWidth + bitsInView - scrollerWidth; // the maximum number previous values stored

boolean enable; // set to true when in use, false when testing gui
