#ifndef __C_AUTOM8_MESSAGE_QUEUE_HPP__
#define __C_AUTOM8_MESSAGE_QUEUE_HPP__

#include <queue>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <base64/base64.h>

#include <autom8/constants.h>
#include <autom8/message/request.hpp>
#include <autom8/message/response.hpp>
#include <autom8/message/message_formatter.hpp>

namespace autom8 {
    /*
     * Utility class used by autom8::session that provides a concurrent queue
     * of pending messages that need to be written to a client. Basically:
     *
     * while (m = message_queue.pop_top()) {
     *   socket.write(m);
     * }
     */
    class message_queue {
    public:
        /*
         * Thrown by pop_next_item() if/when the message queue is shut down
         */
        class stopped_exception {
        };

    private:
        std::queue<message_formatter_ptr> queue_;
        boost::condition_variable wait_for_next_item_condition_;
        boost::mutex queue_mutex_;
        volatile bool active_;

    public:
        message_queue() {
            active_ = true;
        }

        /*
         * Pops the top element from the queue and returns it. If the queue is empty
         * this method will block until there is data to return. If the queue has
         * been stopped, a message_queue::stopped_exception will be thrown.
         */
        message_formatter_ptr pop_top() {
            boost::mutex::scoped_lock lock(queue_mutex_);
            while ((queue_.size() == 0) && (active_ == true)) {
                wait_for_next_item_condition_.wait(lock);
            }

            if ( ! active_) {
                throw stopped_exception();
            }

            message_formatter_ptr top = queue_.front();
            queue_.pop();
            return top;
        }

        /*
         * Pushes the specified message_queue::formatter_ptr to the back of the
         * queue. Returns false if this instance has been stopped, true otherwise.
         */
        bool push(message_formatter_ptr f) {
            boost::mutex::scoped_lock lock(queue_mutex_);

            if (active_) {
                bool was_empty = (queue_.size() == 0);
                queue_.push(f);

                if (was_empty) {
                    wait_for_next_item_condition_.notify_one();
                }

                return true;
            }

            return false;
        }

        /*
         * Clears the the queue and marks this instance as invalid. Any pending or
         * subsequent pop_top() operations will throw a message_queue::shut_down_exception
         * and subsequent push() operations will return false.
         */
        void stop() {
            boost::mutex::scoped_lock lock(queue_mutex_);
            active_ = false;

            while (queue_.size() > 0) {
                queue_.pop();
            }

            wait_for_next_item_condition_.notify_all();
        }
    };

    typedef boost::shared_ptr<message_queue> message_queue_ptr;
} // namespace autom8

#endif