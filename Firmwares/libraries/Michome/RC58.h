#define Run() irsend.sendNEC(0x42BD50AF, 32); delay(100)
#define Input() irsend.sendNEC(0x42BD08F7, 32); delay(100)
#define Mode() irsend.sendNEC(0x42BD48B7, 32); delay(100)
#define VolumeUp() irsend.sendNEC(0x42BDE01F, 32); delay(100)
#define VolumeDown() irsend.sendNEC(0x42BD6897, 32); delay(100)
#define Default() irsend.sendNEC(0x42BD00FF, 32); delay(100)
#define BasUp() irsend.sendNEC(0x42BD906F, 32); delay(100)
#define BasDown() irsend.sendNEC(0x42BDD02F, 32); delay(100)
#define TreblUp() irsend.sendNEC(0x42BD8877, 32); delay(100)
#define TreblDown() irsend.sendNEC(0x42BDC837, 32); delay(100)
#define SurrUp() irsend.sendNEC(0x42BD40BF, 32); delay(100)
#define SurrDown() irsend.sendNEC(0x42BD20DF, 32); delay(100)
#define CenterUp() irsend.sendNEC(0x42BDC03F, 32); delay(100)
#define CenterDown() irsend.sendNEC(0x42BDA05F, 32); delay(100)
#define SWUp() irsend.sendNEC(0x42BDB04F, 32); delay(100)
#define SWDown() irsend.sendNEC(0x42BD10EF, 32); delay(100)