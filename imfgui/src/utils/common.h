#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

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

    static RationalNumber FromIMFString(const std::string &s)
    {
        std::vector<std::string> strs;
        boost::split(strs, s, boost::is_any_of(" "));
        if (strs.size() != 2) {
            return RationalNumber(0, 0);
        }
        return RationalNumber(boost::lexical_cast<int>(strs[0]),
                               boost::lexical_cast<int>(strs[1]));
    }

    std::string AsIMFString() const
    {
        return boost::lexical_cast<std::string>(num) + " " + boost::lexical_cast<std::string>(denum);
    }
};

#endif // COMMON_H_INCLUDED
