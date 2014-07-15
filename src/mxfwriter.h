#ifndef MXFWRITER_H
#define MXFWRITER_H

#include <string>
#include <map>
#include <boost/any.hpp>

class MXFWriter
{
    public:
        MXFWriter(const std::map<std::string, boost::any> &muxerOptions);
        virtual ~MXFWriter();

        void MuxVideoFiles(const std::string& inputDirectory, const std::string &finalFile);
    protected:
    private:

        std::map<std::string, boost::any> _muxerOptions;
};

#endif // MXFWRITER_H
