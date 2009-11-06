// import the serial library
import processing.serial.*;
Serial port;

void setup()
{
  //enable = false; // set to true when in use, false when testing gui
  enable = true;
  // using "if(enable)" allows the GUI to be tested without doing anything with the serial port

  if(enable)
    port = new Serial(this, Serial.list()[0], 115200); // open serial port
  // if Serial.list()[0] doesn't work, try "COM3", or whatever your serial port is named

  // preset all values to 0
  for(int i = 0; i < maxStore; i++)
  {
    if(enable)
      values[i] = 0;
    else
      values[i] = i % 256;
  }
  
  // all channels on
  for(int i = 0; i < numOfChans; i++)
  {
    chanOn[i] = true;
  }
  if(enable)
    port.write(63); // all interrupts on

  size(viewWidth, viewHeight); // open window to size
  
  viewOffset = width; // view start from latest

  noLoop(); // does not loop the draw function
  redraw(); // draw once
}

void serialEvent(Serial p)
{
  // new byte received

  // move buffer
  for (int i = 0; i < maxStore - 1; i++)
  {
    values[i] = values[i + 1];
  }
  if(enable)
    values[maxStore - 1] = byte(port.read()); // place new byte in buffer

  redraw(); // redraw
}

void mouseDragged()
{
  // if scroll bar is moved
  if(mouseY <= height - bottomPadding && mouseY >= height - bottomPadding - scrollBarHeight && mouseX >= scrollerWidth / 2)
  {
    viewOffset = mouseX + (scrollerWidth / 2); // change view
    
    // constrain within limits
    if(viewOffset > width)
    {
      viewOffset = width;
    }
  }

  redraw(); // redraw
}

void mouseClicked()
{
  boolean newMask = false; // variable to see if anything's been changed
  int pcintMask = 0; // new interrupt mask

  // if clicked in side sidebar
  if(mouseX >= 0 && mouseX <= sideBarWidth && mouseY >= topPadding && mouseY <= height - bottomPadding - scrollBarHeight - chanPadding)
  {
    // check each channel
    for(int i = 0; i < numOfChans; i++)
    {
      // if clicked inside this channel
      if(mouseY >= (topPadding  + ((chanHeight + chanPadding) * i)) && mouseY <= (topPadding  + ((chanHeight + chanPadding) * i) + chanHeight))
      {
        // toggle channel status
        if(chanOn[i])
        {
          chanOn[i] = false;
        }
        else
        {
          chanOn[i] = true;
        }

        newMask = true; // a change as been made
      }
      if(chanOn[i])
      {
        pcintMask += pow(2, i); // enable this interrupt if channel is enabled
      }
    }
  }

  // if changes made
  if(newMask)
  {
    if(enable)
      port.write(pcintMask); // send new mask
  }

  redraw(); // redraw
}

void draw()
{
  background(0); // background black
  noFill(); // lines don't have fills
  stroke(255); // white line

  line(sideBarWidth, topPadding, sideBarWidth, height - scrollBarHeight - bottomPadding - chanPadding); // side bar boarder
  line(0, topPadding, width, topPadding); // top boarder

  // for each channel
  for(int i = 0; i < numOfChans; i++)
  {
    // channel boarders
    line(0, topPadding + chanHeight + ((chanHeight + chanPadding) * i), width, topPadding + chanHeight + ((chanHeight + chanPadding) * i));
    line(0, topPadding + chanHeight + chanPadding + ((chanHeight + chanPadding) * i), width, topPadding + chanHeight + chanPadding + ((chanHeight + chanPadding) * i));
  }
  line(0, height - bottomPadding, width, height - bottomPadding); // final channel boarder

  stroke(128); // grey line
  // for each bit in view
  for(int i = 1; i <= bitsInView; i++)
  {
    // this draws the timeline guide for each bit
    line(sideBarWidth + (bitWidth * i), topPadding, sideBarWidth + (bitWidth * i), height - bottomPadding - scrollBarHeight - chanPadding);
  }

  // for each channel
  for(int i = 0; i < numOfChans; i++)
  {
    // if channel enabled, green box, or else, red box
    if(chanOn[i])
    {
      fill(0, 255, 0);
    }
    else
    {
      fill(255, 0, 0);     
    }
    noStroke();
    rectMode(CORNER);
    rect(0, topPadding  + ((chanHeight + chanPadding) * i), sideBarWidth, chanHeight); // draw the box
  }

  // for only the bits in view
  for(int i = viewOffset - bitsInView; i < viewOffset; i++)
  {
    int j = i - (viewOffset - bitsInView);
    
    // for each channel
    for(int k = 0; k < numOfChans; k++)
    {
      // if channel is logic LOW, then draw blue box, else, yellow
      if((floor((values[i + bitsInView - scrollerWidth] / pow(2, k))) % 2) == 0)
      {
        fill(0, 0, 255);
      }
      else
      {
        fill(255, 255, 0);
      }
      rect(sideBarWidth + (bitWidth * j), topPadding + ((chanHeight + chanPadding) * k), bitWidth, chanHeight);
    }
  }

  // green scroll bar scroller box
  fill(0, 255, 0);
  rect(viewOffset - (scrollerWidth), height - bottomPadding - scrollBarHeight, scrollerWidth, scrollBarHeight);  
}
