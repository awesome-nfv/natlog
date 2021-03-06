#include "devicesproducer.ih"

void DevicesProducer::signalHandler(size_t signum)
{
    if (signum == SIGHUP)
    {
        d_stdMsg << "ignoring signal SIGHUP"  << endl;
        return;
    }

    d_stdMsg << "received signal " << signum << " (" << 
        (signum == SIGINT ? "SIGINT" : "SIGTERM") << ')' << endl;

    d_signaled.notify();
}
