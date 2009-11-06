int[] values = new int[maxStore]; // incoming data buffer

boolean[] chanOn = new boolean[numOfChans]; // whether or not if a channel is active
// note, by active, I mean if it's individual interrupt is masked or not

int viewOffset; // scroll bar variable
