 byte Dozd[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x02,
  0x08,
  0x02
};

 byte groza[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x04,
  0x06,
  0x02,
  0x02
};

 byte oblazn[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x00,
  0x00,
  0x00
};

 byte soln[] = {
  0x00,
  0x00,
  0x04,
  0x0E,
  0x1F,
  0x0E,
  0x04,
  0x00
};

 byte sneg[] = {
  0x04,
  0x0A,
  0x11,
  0x0E,
  0x00,
  0x04,
  0x0A,
  0x04
};

extern byte gradus[] = {
  0x18,
  0x18,
  0x03,
  0x04,
  0x04,
  0x04,
  0x03,
  0x00
};

 byte watchh[] = {
  0x0E,
  0x15,
  0x15,
  0x15,
  0x17,
  0x11,
  0x11,
  0x0E
};

extern byte homes[] = {
  0x00,
  0x00,
  0x04,
  0x0E,
  0x1F,
  0x1F,
  0x1B,
  0x1B
};

int IDtoIcon(int id) {
  if (id == 0) {
    return 2;
  }
  else if (id == 1) {
    return 0;
  }
  else if (id == 2) {
    return 0;
  }
  else if (id == 3) {
    return 4;
  }
  else if (id == 4) {
    return 3;
  }
  else if (id == 5) {
    return 1;
  }
  else {
    return 5;
  }
}

void ToHomes(){
  lcd.createChar(0, homes);
  lcd.createChar(1, watchh);
  lcd.createChar(2, groza);
  lcd.createChar(3, soln);
  lcd.createChar(4, sneg);
  lcd.createChar(5, gradus);
}

void ToPrognoz(){
  lcd.createChar(0, Dozd);
  lcd.createChar(1, oblazn);
  lcd.createChar(2, groza);
  lcd.createChar(3, soln);
  lcd.createChar(4, sneg);
  lcd.createChar(5, gradus);
}
