#ifndef RATIONALNUMBER_H_INCLUDED
#define RATIONALNUMBER_H_INCLUDED

struct RationalNumber {
    RationalNumber(int n, int d) : num(n), denum(d) {}
    RationalNumber() : num(0), denum(0) {}
    int num;
    int denum;
};

#endif // RATIONALNUMBER_H_INCLUDED
