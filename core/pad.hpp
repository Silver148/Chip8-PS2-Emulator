#ifndef PAD_H
#define PAD_H

class PAD{
public:
    int waitPadReady(int port, int slot);
    int initializePad(int port, int slot);
};

#endif