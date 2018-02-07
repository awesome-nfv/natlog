#ifndef INCLUDED_ROTATABLESTREAMBUF_
#define INCLUDED_ROTATABLESTREAMBUF_

#include <streambuf>
#include <fstream>
#include <vector>
#include <iosfwd>
#include <mutex>
#include <bobcat/semaphore>

class RotatingStreambuf: public std::streambuf
{
    std::mutex d_mutex;
    bool d_locked = false;
    volatile bool d_contents = false;

    std::ofstream d_out;
    std::string d_name;

    static std::vector<RotatingStreambuf *> s_rotate;
    static FBB::Semaphore s_semaphore;
  
    int (RotatingStreambuf::*d_overflow)(int ch);
    void (*d_header)(std::ostream &);

    public:
        RotatingStreambuf(void (*header)(std::ostream &) = 0);
        void open(std::string const &name);
        static void notify();

    private:
        int overflow(int ch) override;
        int sync() override;

        int unlockedOverflow(int ch);
        int lockedOverflow(int ch);

        static void rotateThread();
        void rotate(size_t nFiles);
};

inline RotatingStreambuf::RotatingStreambuf(void (*header)(std::ostream &))
:
    d_header(header)
{}

inline void RotatingStreambuf::notify()
{
    s_semaphore.notify();
}
       
#endif
