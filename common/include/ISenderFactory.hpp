#ifndef ISENDER_FACTORY_HPP_
#define ISENDER_FACTORY_HPP_

class ISenderFactory {
    public:
        virtual ~ISenderFactory() = default;

    protected:
        virtual DBusMessage* makeMsg(const char *objectpath, const char *interface,
                                    const char* signal, DBusCommand cmd) = 0;
};

#endif // ISENDER_FACTORY_HPP_