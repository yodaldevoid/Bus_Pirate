#ifndef BP_SUMP_H
#define BP_SUMP_H

void SUMP(void);
void SUMPreset(void);
unsigned char SUMPlogicCommand(unsigned char inByte); // N/A JTR SUMP MODE
unsigned char SUMPlogicService(void);

#endif /* BP_SUMP_H */