#pragma once

#include <ev++.h>

#include <f8n/runtime/MessageQueue.h>

#include <iostream>

static const short EVENT_DISPATCH = 1;
static const short EVENT_QUIT = 2;
static int pipeFd[2] = { 0 };

using namespace f8n::runtime;

class EvMessageQueue: public MessageQueue {
    public:
        void Post(IMessagePtr message, int64_t delayMs) {
            MessageQueue::Post(message, delayMs);

            if (delayMs <= 0) {
                write(pipeFd[1], &EVENT_DISPATCH, sizeof(EVENT_DISPATCH));
            }
            else {
                double delayTs = (double) delayMs / 1000.0;
                loop.once<
                    EvMessageQueue,
                    &EvMessageQueue::DelayedDispatch
                >(-1, ev::TIMER, (ev::tstamp) delayTs, this);
            }
        }

        void DelayedDispatch(int revents) {
            this->Dispatch();
        }

        static void SignalQuit(ev::sig& signal, int revents) {
            write(pipeFd[1], &EVENT_QUIT, sizeof(EVENT_QUIT));
        }

        void ReadCallback(ev::io& watcher, int revents) {
            short type;
            if (read(pipeFd[0], &type, sizeof(type)) == 0) {
                std::cerr << "read() failed.\n";
                exit(EXIT_FAILURE);
            }
            switch (type) {
                case EVENT_DISPATCH: this->Dispatch(); break;
                case EVENT_QUIT: loop.break_loop(ev::ALL); break;
            }
        }

        void Run() {
            io.set(loop);
            io.set(pipeFd[0], ev::READ);
            io.set<EvMessageQueue, &EvMessageQueue::ReadCallback>(this);
            io.start();

            sio.set(loop);
            sio.set<&EvMessageQueue::SignalQuit>();
            sio.start(SIGTERM);

            write(pipeFd[1], &EVENT_DISPATCH, sizeof(EVENT_DISPATCH));

            loop.run(0);
        }

    private:
        ev::dynamic_loop loop;
        ev::io io;
        ev::sig sio;
};