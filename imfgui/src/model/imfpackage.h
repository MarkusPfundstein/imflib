#ifndef IMFPACKAGE_H
#define IMFPACKAGE_H


class IMFPackage
{
    public:
        IMFPackage();
        virtual ~IMFPackage();

        IMFPackage(const IMFPackage& other) = delete;
        IMFPackage& operator=(const IMFPackage& other) = delete;
    protected:
    private:
};

#endif // IMFPACKAGE_H
