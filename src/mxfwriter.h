#ifndef MXFWRITER_H
#define MXFWRITER_H

#include <string>
#include <map>
#include <boost/any.hpp>
#include <list>

class MXFWriter
{
    public:
        MXFWriter(const std::map<std::string, boost::any> &muxerOptions);
        virtual ~MXFWriter();

        void MuxAudioFile(const std::string& file, const std::string &finalFile);
        void MuxVideoFiles(const std::list<std::string> &files, const std::string &finalFile);
    protected:
    private:

        std::map<std::string, boost::any> _muxerOptions;
};

#endif // MXFWRITER_H
