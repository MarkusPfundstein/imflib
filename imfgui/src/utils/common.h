#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

struct RationalNumber {
    RationalNumber(int n, int d) : num(n), denum(d) {}
    RationalNumber() : num(0), denum(0) {}
    int num;
    int denum;

    double AsDouble() const
    {
        if (denum == 0) return 0.0;
        return (double) num / denum;
    }

    float AsFloat() const
    {
        if (denum == 0) return 0.0f;
        return (float) num / denum;
    }
};

#endif // COMMON_H_INCLUDED
