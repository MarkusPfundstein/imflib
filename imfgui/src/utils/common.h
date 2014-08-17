#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <string>
#include <boost/lexical_cast.hpp>

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

    std::string AsIMFString() const
    {
        return boost::lexical_cast<std::string>(num) + " " + boost::lexical_cast<std::string>(denum);
    }
};

static const std::string XML_HEADER_ISSUER("ContentCoders (Research and Development)");
static const std::string XML_HEADER_CREATOR("ODMedia IMF Suite dev-0.1");
static const std::string XML_HEADER_ANNOTATION("For testing purposes only");

#endif // COMMON_H_INCLUDED
