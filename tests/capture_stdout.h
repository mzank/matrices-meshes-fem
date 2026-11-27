/**
 * @file capture_stdout.h
 * @brief RAII helper for temporarily redirecting std::cout in tests.
 */

#ifndef CAPTURE_STDOUT_H_INCLUDED
#define CAPTURE_STDOUT_H_INCLUDED

#include <iostream>
#include <streambuf>

namespace mmf_test {
/**
 * @class StdoutCapture
 * @brief Redirects std::cout into a target stream for the lifetime of the guard.
 *
 * The original stream buffer is restored by the destructor, so a failed
 * assertion (which throws) cannot leave stdout redirected.
 */
class StdoutCapture {
public:
    explicit StdoutCapture(std::ostream& target)
        : original_buffer_(std::cout.rdbuf(target.rdbuf())) {}

    StdoutCapture(StdoutCapture const&) = delete;
    StdoutCapture& operator=(StdoutCapture const&) = delete;

    ~StdoutCapture() {
        std::cout.rdbuf(original_buffer_);
    }

private:
    std::streambuf* original_buffer_;
};
} // namespace mmf_test

#endif // CAPTURE_STDOUT_H_INCLUDED
