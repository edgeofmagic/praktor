# praktor
Asynchronous I/O and event handling, based on libuv

Praktor is a C++ wrapper for libuv. Praktor supports asynchronous event handling based on the proactor design pattern[ref].
An asynchronous program's control flow consists of two kinds of actions--initiating an asynchronous operation, 
and handling the completion of an asynchronous operation. In this context, an <i>asychnronous operation</i> is some action that the system undertakes on behalf of the application that is not CPU-bound, such as reading from (or writing to) a socket.

In conventional synchronous programs, such operations cause the calling thread to block until the operation is complete. While blocked, the calling thread can perform no work. If there is work to be done that doesn't depend on the write completion, it must be done in another thread.

Asynchronous programs are decomposed into units of code that are executed in response to the completion of asynchronous operations. When the program initiates an asynchronous action, it provides a separate unit of code that will be executed when the operation completes, called (unsurprisingly) a completion handler.

Here is a simple example program that connects to a socket, writes a message, and reads a response:

```` cpp
#include <praktor/praktor.h>

using namespace praktor;

int main()
{
    auto lp = loop::create_loop();
    ip::endpoint ep{ip::address::v4_loopback(), 7001};

    lp->connect_channel(ep, 
    // The following lambda expression is the completion
    // handle for the connect operation. It is executed by
    // the event loop when the connection is made, or when
    // an error occurs.
    [=](channel::ptr const& sock, std::error_code const& err) 
    {
        if (!err)
        {
            sock->start_write(mutable_buffer{"hello"},
                // Likewise, the following lambda expression is
                // the completion handler for the write operation.
                [=](channel::ptr const& sock, 
                    mutable_buffer&& outbuf, 
                    std::error_code const& err)
                {
                    if (!err)
                    {
                        sock->start_read(
                            // And so on.
                            [=](channel::ptr const& sock,
                                const_buffer&& inbuf,
                                std::error_code const& err)
                            {
                                std::cout << inbuf.to_string() << std::endl;
                                sock->loop()->stop();
                            });
                    }
                });
        }
    });

    lp->run();
}
````
As do most beginning examples, this program has some deficiencies due to its simplicity in the interest of brevity. To wit:

There is no error handling. Don't do this in real life. Praktor uses std::error_code as its primary mechanism for reporting errors, so please
make good use of it.

The program doesn't take into account that there are no explicit message boundaries with stream-based sockets. Specifically, when this program
writes the string "hello", there is no guarantee that the program reading
from the peer socket will receive a single buffer with the same contents. It's possible (although unlikely) that a read operation on the peer socket will return only a fragment of the buffer as it was written. It's up the the application to determine what may or may not constitute message boundaries, and how they should be enforced. (See framing below.)

Some basic observations

Most asychronous programs follow a simple pattern: the main program creates an event loop, initiates one or more asynchronous operations, providing completion handlers for those operations, and starts the event loop.

The completion handlers perform no blocking operations. In many cases, they may initiate additional asynchronous operations, providing an appropriate completion handler for each one. When the handler is finished, it simply returns (thus, returning control to the event loop).

As a general rule, the event loop and all of the completion handlers it invokes run in the same thread. Each completion handler can be regarded as an atomic unit. Completion handlers are executed sequentially, the order being determined by the event loop. There is no need for mutexes or other explicit synchronization mechanisms.

## Questions I Might Be Asking If I Were You

<b><i>Does this mean I can't use multiple threads? Wouldn't this mean I'm only using one core of my multi-core processor?</i></b>

No, and not necessarily. Asynchronous event loops and multi-threaded programming are not mutually exclusive. They can be used together successfully if you adopt a clear design strategy and observe some basic rules.

Some example design strategies

Mulitiple event loops, one thread per loop.

This strategy is easy to implement, as far as mechanics are concerned. Each thread has the same basic structure as the main function of a single-loop program: create a loop, initiate actions (providing completion handlers), and run the loop. The key to success with this strategy is the distribution of work among the threads/loops, which can only be determined by understanding the desired behavior of the application. 

One event loop driving multiple worker threads.

This strategy is applicable if the computation/event ratio is relatively high, that is, each event processed by the asynchronous loop entails a significant amount of computation that can be performed as a non-blocking unit. 



