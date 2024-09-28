#ifndef symbols_inf_h
#define symbols_inf_h
static byte Dozd[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x0A,
  0x14,
  0x00
};

static byte LittleDozd[] = {
  0x00,
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x08,
  0x02
};

static byte groza[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x04,
  0x06,
  0x02,
  0x02
};

static byte oblazn[] = {
  0x00,
  0x00,
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x00
};

static byte soln[] = {
  0x00,
  0x00,
  0x04,
  0x0E,
  0x1F,
  0x0E,
  0x04,
  0x00
};

static byte sneg[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x04,
  0x0A,
  0x04
};

static byte gradus[] = {
  0x18,
  0x18,
  0x03,
  0x04,
  0x04,
  0x04,
  0x03,
  0x00
};

static byte watchh[] = {
  0x0E,
  0x15,
  0x15,
  0x15,
  0x17,
  0x11,
  0x11,
  0x0E
};

static byte homes[] = {
  0x00,
  0x00,
  0x04,
  0x0E,
  0x1F,
  0x1F,
  0x1B,
  0x1B
};

static byte clockI[] = {
  0x00,
  0x0E,
  0x13,
  0x15,
  0x11,
  0x0E,
  0x00,
  0x00
};
static byte Delimiters[] = {
  0x04,
  0x04,
  0x04,
  0x04,
  0x04,
  0x04,
  0x04,
  0x04
};

static byte Minus[] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x07,
  0x00,
  0x00,
  0x00
};

static byte WindUp[] = {
  0x00,
  0x04,
  0x0E,
  0x15,
  0x04,
  0x04,
  0x00,
  0x00
};

static byte WindDown[] = {
  0x00,
  0x04,
  0x04,
  0x15,
  0x0E,
  0x04,
  0x00,
  0x00
};

static byte WindLeft[] = {
  0x00,
  0x04,
  0x08,
  0x1F,
  0x08,
  0x04,
  0x00,
  0x00
};

static byte WindRight[] = {
  0x00,
  0x04,
  0x02,
  0x1F,
  0x02,
  0x04,
  0x00,
  0x00
};

static byte WindUpRight[] = {
  0x00,
  0x0F,
  0x03,
  0x05,
  0x09,
  0x10,
  0x00,
  0x00
};

static byte WindUpLeft[] = {
  0x1E,
  0x18,
  0x14,
  0x12,
  0x01,
  0x00,
  0x00,
  0x00
};

static byte WindDownLeft[] = {
  0x00,
  0x00,
  0x00,
  0x01,
  0x12,
  0x14,
  0x18,
  0x1E
};

static byte WindDownRight[] = {
  0x00,
  0x00,
  0x00,
  0x11,
  0x09,
  0x05,
  0x03,
  0x0F
};

static byte WindNone[] = {
  0x00,
  0x00,
  0x04,
  0x0A,
  0x0A,
  0x04,
  0x00,
  0x00
};

static int IDtoIcon(int id) {
  if (id == 0)
    return 2;
  else if (id == 1)
    return 6;
  else if (id == 2)
    return 0;
  else if (id == 3)
    return 4;
  else if (id == 4)
    return 3;
  else if (id == 5)
    return 1;
  else if (id == 6)
    return 6;
  else if (id == 7)
    return 7;
  else
    return 5;
}

static byte* IDtoSymbol(int id) {
  if (id == 0)
    return groza;
  else if (id == 1)
    return LittleDozd;
  else if (id == 2)
    return Dozd;
  else if (id == 3)
    return sneg;
  else if (id == 4)
    return soln;
  else if (id == 5)
    return oblazn;
  else if (id == 6)
    return LittleDozd;
  else if (id == 7)
    return clockI;
  else
    return groza;
}

static byte* WindIDtoSymbol(int id) {
  if (id == 0)
    return WindNone;
  else if (id == 1)
    return WindUp;
  else if (id == 2)
    return WindUpRight;
  else if (id == 3)
    return WindRight;
  else if (id == 4)
    return WindDownRight;
  else if (id == 5)
    return WindDown;
  else if (id == 6)
    return WindDownLeft;
  else if (id == 7)
    return WindLeft;
  else if (id == 8)
    return WindUpLeft;
  else if (id == 9)
    return WindUp;
  else
    return WindNone;
}
#endif