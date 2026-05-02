#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <complex>
#include <iomanip>

namespace nspace
{
    struct DataStruct
    {
        unsigned long long key1;
        std::complex<double> key2;
        std::string key3;

        bool operator<(const DataStruct& other) const
        {
            if (key1 != other.key1)
            {
                return key1 < other.key1;
            }
            if (std::abs(key2) != std::abs(other.key2))
            {
                return std::abs(key2) < std::abs(other.key2);
            }
            return key3.length() < other.key3.length();
        }
    };

    struct DelimiterIO { char exp; };
    struct OctValueIO { unsigned long long& ref; };
    struct ComplexValueIO { std::complex<double>& ref; };
    struct StringValueIO { std::string& ref; };
    struct LabelIO { std::string exp; };

    std::istream& operator>>(std::istream& in, DelimiterIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;
        char c = '0';
        in >> c;
        if (in && (tolower(c) != tolower(dest.exp))) in.setstate(std::ios::failbit);
        return in;
    }

    std::istream& operator>>(std::istream& in, OctValueIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;
        return in >> std::oct >> dest.ref;
    }

    std::istream& operator>>(std::istream& in, ComplexValueIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;
        double re = 0.0, im = 0.0;
        in >> DelimiterIO{ '#' } >> DelimiterIO{ 'c' } >> DelimiterIO{ '(' } >> re >> im >> DelimiterIO{ ')' };
        if (in) dest.ref = { re, im };
        return in;
    }

    std::istream& operator>>(std::istream& in, StringValueIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;
        return std::getline(in >> DelimiterIO{ '"' }, dest.ref, '"');
    }

    std::istream& operator>>(std::istream& in, LabelIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;
        std::string data;
        for (size_t i = 0; i < dest.exp.length(); ++i)
        {
            char c;
            in >> c;
            data += c;
        }
        if (in && (data != dest.exp)) in.setstate(std::ios::failbit);
        return in;
    }

    std::istream& operator>>(std::istream& in, DataStruct& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry) return in;

        DataStruct input;
        in >> DelimiterIO{ '(' } >> DelimiterIO{ ':' };

        for (int i = 0; i < 3; ++i)
        {
            std::string key;
            std::getline(in, key, ' ');
            if (key == "key1") in >> OctValueIO{ input.key1 };
            else if (key == "key2") in >> ComplexValueIO{ input.key2 };
            else if (key == "key3") in >> StringValueIO{ input.key3 };

            in >> DelimiterIO{ ':' };
        }
        in >> DelimiterIO{ ')' };

        if (in) dest = input;
        return in;
    }

    std::ostream& operator<<(std::ostream& out, const DataStruct& src)
    {
        std::ostream::sentry sentry(out);
        if (!sentry) return out;
        out << "(:key1 0" << std::oct << src.key1;
        out << ":key2 #c(" << std::fixed << std::setprecision(1) << src.key2.real() << " " << src.key2.imag() << ")";
        out << ":key3 \"" << src.key3 << "\":)";
        out << std::dec;
        out.unsetf(std::ios_base::fixed);
        return out;
    }
}

int main()
{
    using namespace nspace;
    std::vector<DataStruct> data;
    std::copy(
        std::istream_iterator<DataStruct>(std::cin),
        std::istream_iterator<DataStruct>(),
        std::back_inserter(data)
    );
    std::sort(data.begin(), data.end());
    std::copy(
        data.begin(),
        data.end(),
        std::ostream_iterator<DataStruct>(std::cout, "\n")
    );

    return 0;
}
