/* *****************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Vasiliy V. Bodrov aka Bodro, Ryazan, Russia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ************************************************************************** */

/* *****************************************************************************
 * Program name: testserver
 * Description: It's a singlthread network server for testing
 * Version: 1.0
 * Date: February-March, 2018
 * Author: Vasiliy V. Bodrov aka Bodro (also called IPBSoftware or IPBS)
 *         +7 (930) 783-0-783 (Russia)
 * --------------------------------------------------------------------------
 * Note: you can define these macroses:
 *       DEBUG
 *       TCP_PORT [INT]
 *       BUFFER_SIZE [INT]
 *       POLL_TIMEOUT[INT]
 *       MAX_SOCKET_COUNT[INT]
 *       TEST_FILENAME [STRING]
 *       USE_RANDOM_LINGER_RST [INT]
 *       FORCE_LINGER_RST[{0, 1, 2}]
 *       USE_RANDOM_KEEP_ALIVE [INT]
 *       FORCE_KEEP_ALIVE[{0, 1, 2}]
 *       USE_RANDOM_TCP_NO_DELAY [INT]
 *       FORCE_TCP_NO_DELAY[{0, 1, 2}]
 * ************************************************************************** */

#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ios>
#include <iomanip>
#include <memory>
#include <new>
#include <chrono>
#include <random>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <boost/cstdint.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef TCP_PORT
    #define TCP_PORT 7777
#endif // TCP_PORT

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 3000
#endif // BUFFER_SIZE

#ifndef POLL_TIMEOUT
    #define POLL_TIMEOUT 1000
#endif // POLL_TIMEOUT

#ifndef MAX_SOCKET_COUNT
    #define MAX_SOCKET_COUNT 200
#endif // MAX_SOCKET_COUNT

#ifndef TEST_FILENAME
    #define TEST_FILENAME "test.bin"
#endif // TEST_FILENAME

#ifndef USE_RANDOM_LINGER_RST
    #define USE_RANDOM_LINGER_RST 1
#endif // USE_RANDOM_LINGER_RST

#ifndef USE_RANDOM_KEEP_ALIVE
    #define USE_RANDOM_KEEP_ALIVE 1
#endif // USE_RANDOM_KEEP_ALIVE

#ifndef USE_RANDOM_TCP_NO_DELAY
    #define USE_RANDOM_TCP_NO_DELAY 1
#endif // USE_RANDOM_TCP_NO_DELAY

#if defined(FORCE_LINGER_RST) && (USE_RANDOM_LINGER_RST != 1)
    #error You need to define the macros USE_RANDOM_LINGER_RST!
#endif // if defined(FORCE_LINGER_RST) && (USE_RANDOM_LINGER_RST != 1)

#if defined(FORCE_KEEP_ALIVE) && (USE_RANDOM_KEEP_ALIVE != 1)
    #error You need to define the macros USE_RANDOM_KEEP_ALIVE!
#endif // if defined(FORCE_KEEP_ALIVE) && (USE_RANDOM_KEEP_ALIVE != 1)

#if defined(FORCE_TCP_NO_DELAY) && (USE_RANDOM_TCP_NO_DELAY != 1)
    #error You need to define the macros USE_RANDOM_TCP_NO_DELAY!
#endif // if defined(FORCE_TCP_NO_DELAY) && (USE_RANDOM_TCP_NO_DELAY != 1)

#if !(FORCE_LINGER_RST == 0 || FORCE_LINGER_RST == 1 || FORCE_LINGER_RST == 2)
    #error The macros FORCE_LINGER_RST can be 0, 1 or 2!
#endif

#if !(FORCE_KEEP_ALIVE == 0 || FORCE_KEEP_ALIVE == 1 || FORCE_KEEP_ALIVE == 2)
    #error The macros FORCE_KEEP_ALIVE can be 0, 1 or 2!
#endif

#if !(FORCE_TCP_NO_DELAY == 0 || FORCE_TCP_NO_DELAY == 1 || \
      FORCE_TCP_NO_DELAY == 2)
    #error The macros FORCE_TCP_NO_DELAY can be 0, 1 or 2!
#endif

//
// It's the main function in program (enter point to this program)
//
int main(void) {
    int listen_sd = -1;
    int new_sd = -1;
    unsigned char buffer[BUFFER_SIZE] = { 0 };
    int rc = 0;
    int on = 1;
    int len = 0;
    struct pollfd fds[2 * MAX_SOCKET_COUNT + 1];
    int nfds = 0;
    int timeout = POLL_TIMEOUT;
    int flags = 0;
    struct sockaddr_in addr;
    int port = TCP_PORT;
    bool compress_array = false;
    int fd = 0;
    int global_count = 0;
    std::map<int, boost::uint32_t> counter_sent;
    std::map<int, boost::uint32_t> counter_recv;
    std::map<int, std::chrono::system_clock::time_point> start_time_work;

    auto print_nfds = [&]() {
        for(int i = 0; i < nfds; i++) {
            std::cout << i << ": " << fds[i].fd << std::endl;
        }
    };

    auto close_socket = [&](int i) {
        (void) ::close(fds[i].fd);
        (void) ::close(fds[i + 1].fd);
        counter_sent.erase(fds[i].fd);
        counter_recv.erase(fds[i].fd);
        start_time_work.erase(fds[i].fd);
        fds[i].fd = -1;
        fds[i + 1].fd = -1;
        compress_array = true;
    };

    auto close_file = [&](int i) {
        (void) ::close(fds[i].fd);
        (void) ::close(fds[i - 1].fd);
        counter_sent.erase(fds[i - 1].fd);
        counter_recv.erase(fds[i - 1].fd);
        start_time_work.erase(fds[i - 1].fd);
        fds[i].fd = -1;
        fds[i - 1].fd = -1;
        compress_array = true;
    };

    auto testfilesize = []() -> std::ifstream::pos_type {
        std::ifstream in(TEST_FILENAME,
                         std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    };

    auto get_time_interval_mcsec = [&](int sd, auto end_time) {
        auto start_time = start_time_work[sd];
        auto dur = std::chrono::duration_cast<
                std::chrono::microseconds>(
                    end_time - start_time).count();
        return dur;
    };

    auto get_time_interval_ms = [&](int sd, auto end_time) {
        auto start_time = start_time_work[sd];
        auto dur = std::chrono::duration_cast<
                std::chrono::milliseconds>(
                    end_time - start_time).count();
        return dur;
    };

    auto get_time_interval_sec = [&](int sd, auto end_time) {
        auto start_time = start_time_work[sd];
        auto dur = std::chrono::duration_cast<
                std::chrono::seconds>(
                    end_time - start_time).count();
        return dur;
    };

    auto get_str_info = [&](int sd, auto end_time) -> std::string {
        std::stringstream ss;
        std::string ident = "\t";
        double speed_sent = 0.0;
        double speed_recv = 0.0;
        double speed_sent_bit = 0.0;
        double speed_recv_bit = 0.0;
        auto interval_mcs = get_time_interval_mcsec(sd, end_time);
        auto interval_ms = get_time_interval_ms(sd, end_time);
        auto interval_sec = get_time_interval_sec(sd, end_time);

        speed_sent = (static_cast<double>(counter_sent[sd]) /
                static_cast<double>(interval_ms)) * 1000.0 / 1024 / 1024;
        speed_recv = (static_cast<double>(counter_recv[sd]) /
                static_cast<double>(interval_ms)) * 1000.0 / 1024 / 1024;
        speed_sent_bit = speed_sent * 8;
        speed_recv_bit = speed_recv * 8;

        ss << std::endl
           << ident
           << "Bytes sent: "
           << counter_sent[sd]
           << "; bytes recv: "
           << counter_recv[sd]
           << "." << std::endl
           << ident
           << "Time: "
           << interval_mcs
           << " mcs; ~"
           << interval_ms
           << " ms; ~"
           << interval_sec
           << " sec."
           << std::endl
           << ident
           << "Speed sent: "
           << std::to_string(speed_sent)
           << " MBytes/sec; "
           << "speed recv: "
           << std::to_string(speed_recv)
           << " MBytes/sec."
           << std::endl
           << ident
           << "Speed sent: "
           << std::to_string(speed_sent_bit)
           << " MBits/sec; "
           << "speed recv: "
           << std::to_string(speed_recv_bit)
           << " MBits/sec.";

        return ss.str();
    };

    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "LESTEN TCP-PORT: " << port << std::endl;
    std::cout << "TEST FILENAME: " << TEST_FILENAME << std::endl;
    std::cout << "FILESIZE: " << testfilesize() << " bytes" << std::endl;
    std::cout << "DEBUG MODE: ";
#ifdef DEBUG
    std::cout << "ON";
#else // DEBUG
    std::cout << "OFF";
#endif // DEBUG
    std::cout << std::endl;
    std::cout << "POLL TIMEOUT: " << POLL_TIMEOUT << " ms" << std::endl;
    std::cout << "BUFFER SIZE: " << BUFFER_SIZE << " bytes" << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;

    listen_sd = ::socket(AF_INET, SOCK_STREAM, 0);
    rc = ::setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR,
                      reinterpret_cast<char*>(&on), sizeof(on));
    assert(rc >= 0);

    rc = ::ioctl(listen_sd, FIONBIO, reinterpret_cast<char*>(&on));
    assert(rc >= 0);

    flags = ::fcntl(listen_sd, F_GETFL, 0);
    rc = ::fcntl(listen_sd, F_SETFL, flags | O_NONBLOCK);
    assert(rc >= 0);

    std::fill_n(reinterpret_cast<char*>(&addr), sizeof(addr), '\0');

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    rc = ::bind(listen_sd,
                reinterpret_cast<struct sockaddr*>(&addr),
                sizeof(addr));
    assert(rc >= 0);

    rc = ::listen(listen_sd, 1);
    assert(rc >= 0);

    std::fill_n(reinterpret_cast<char*>(fds), sizeof(fds), '\0');

    nfds = 0;

    fds[nfds].fd = listen_sd;
    fds[nfds].events = POLLIN;
    nfds++;

    do {
        rc = ::poll(fds, nfds, timeout);
        assert(rc >= 0);

        if(0 == rc) {
#if defined(DEBUG)
            std::cout << ">>> Time is up (poll)..." << std::endl;
#endif // DEBUG
            continue;
        }
        else {
            int current_size = nfds;
            for(int i = 0; i < current_size; i++) {
                if(0 == fds[i].revents) {
                    continue;
                }
                else if(!((fds[i].revents & POLLIN) ||
                          (fds[i].revents & POLLOUT) ||
                          (fds[i].revents & POLLHUP) ||
                          (fds[i].revents & POLLERR))) {
                    std::cout << ">>> Unknown event! Exit! ("
                              << "revents = 0x" << std::hex << fds[i].revents
                              << std::dec << " [" << fds[i].revents << "]"
                              << ")" << std::endl;
                    ::exit(EXIT_SUCCESS);
                }
                else if((fds[i].revents & POLLHUP) ||
                        (fds[i].revents & POLLERR)) {
                            std::cout << ">>> Event ("
                                      << "revents = 0x" << std::hex
                                      << fds[i].revents
                                      << std::dec << " [" << fds[i].revents
                                      << "]"
                                      << ") is POLLHUP or POLLERR."
                                      << std::endl;
                    if(i % 2) {
                        // Socket
                        std::cout << "> "
                                  << "Connection close due to error (socket): "
                                  << fds[i].fd << ":"
                                  << fds[i + 1].fd
                                  << "."
                                  << get_str_info(fds[i].fd,
                                        std::chrono::system_clock::now())
                                  << std::endl;
                        close_socket(i);
                    }
                    else {
                        // File
                        std::cout << "> "
                                  << "Connection closed due to error (file): "
                                  << fds[i - 1].fd << ":"
                                  << fds[i].fd
                                  << "."
                                  << get_str_info(fds[i - 1].fd,
                                        std::chrono::system_clock::now())
                                  << std::endl;
                        close_file(i);
                    }

                    break;
                }
                else {
                    if(fds[i].fd == -1) {
                        continue;
                    }
                    else if(fds[i].fd == listen_sd) {
                        if(nfds < (2 * MAX_SOCKET_COUNT + 1)) {
                            struct sockaddr_in client_addr;
                            socklen_t client_addr_len = sizeof(client_addr);

                            std::fill_n(reinterpret_cast<char*>(&client_addr),
                                        client_addr_len, '\0');

                            new_sd = ::accept(listen_sd,
                                          reinterpret_cast<struct sockaddr*>(
                                              &client_addr), &client_addr_len);
                            assert(new_sd >= 0);

                            fd = ::open(TEST_FILENAME, O_RDONLY);
                            assert(fd >= 0);

                            std::cout << "> "
                                      << "New connection: "
                                      << new_sd
                                      << ":"
                                      << fd
                                      << " ("
                                      << "sd=" << new_sd << "; fd=" << fd
                                      << "). Client: "
                                      << inet_ntoa(client_addr.sin_addr)
                                      << ":"
                                      << ntohs(client_addr.sin_port)
                                      << "."
                                      << std::endl;

                            on = 1;
                            rc = ::ioctl(new_sd, FIONBIO,
                                         reinterpret_cast<char*>(&on));
                            assert(rc >= 0);

                            flags = ::fcntl(new_sd, F_GETFL, 0);
                            rc = ::fcntl(new_sd, F_SETFL, flags | O_NONBLOCK);
                            assert(rc >= 0);

#if defined(USE_RANDOM_KEEP_ALIVE) && USE_RANDOM_KEEP_ALIVE == 1
                            do {
                                std::random_device rd;
                                std::mt19937 gen(rd());
                                std::uniform_int_distribution<> dis(0, 2);
                                int keep_alive_value;
                                socklen_t keep_alive_value_size =
                                        sizeof(keep_alive_value);
                                int res_gen = dis(gen);

    #if defined(USE_RANDOM_KEEP_ALIVE)
        #if USE_RANDOM_KEEP_ALIVE == 0
                                res_gen = 0;
        #elif USE_RANDOM_KEEP_ALIVE == 1
                                res_gen = 1;
        #else // USE_RANDOM_KEEP_ALIVE == 2
                                res_gen = 2;
        #endif // USE_RANDOM_KEEP_ALIVE
    #endif // USE_RANDOM_KEEP_ALIVE

                                if(0 == res_gen) {
                                    keep_alive_value = 1;
                                    rc = ::setsockopt(new_sd, SOL_SOCKET,
                                                      SO_KEEPALIVE,
                                                      &keep_alive_value,
                                                      keep_alive_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'KEEPALIVE' "
                                              << "is ON ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                                else if(1 == res_gen) {
                                    keep_alive_value = 0;
                                    rc = ::setsockopt(new_sd, SOL_SOCKET,
                                                      SO_KEEPALIVE,
                                                      &keep_alive_value,
                                                      keep_alive_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'KEEPALIVE' "
                                              << "is OFF ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                                else {
                                    std::cout << "NOTE: 'KEEPALIVE' "
                                              << "by default ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                            }
                            while(false);
#else // USE_RANDOM_KEEP_ALIVE
                            std::cout << "NOTE: 'KEEPALIVE' "
                                      << "by default ("
                                      << "socket = " << new_sd << ")."
                                      << std::endl;
#endif // USE_RANDOM_KEEP_ALIVE

#if defined(USE_RANDOM_TCP_NO_DELAY) && USE_RANDOM_TCP_NO_DELAY == 1
//        FORCE_TCP_NO_DELAY
                            do {
                                std::random_device rd;
                                std::mt19937 gen(rd());
                                std::uniform_int_distribution<> dis(0, 2);
                                int tcp_no_delay_value;
                                socklen_t tcp_no_delay_value_size =
                                        sizeof(tcp_no_delay_value);
                                int res_gen = dis(gen);

    #if defined(FORCE_TCP_NO_DELAY)
        #if FORCE_TCP_NO_DELAY == 0
                                res_gen = 0;
        #elif FORCE_TCP_NO_DELAY == 1
                                res_gen = 1;
        #else // FORCE_TCP_NO_DELAY == 2
                                res_gen = 2;
        #endif // FORCE_TCP_NO_DELAY
    #endif // FORCE_TCP_NO_DELAY

                                if(0 == res_gen) {
                                    tcp_no_delay_value = 1;
                                    rc = ::setsockopt(new_sd, IPPROTO_TCP,
                                                      TCP_NODELAY,
                                                      &tcp_no_delay_value,
                                                      tcp_no_delay_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'TCP_NODELAY' "
                                              << "is ON ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                                else if(1 == res_gen) {
                                    tcp_no_delay_value = 0;
                                    rc = ::setsockopt(new_sd, IPPROTO_TCP,
                                                      TCP_NODELAY,
                                                      &tcp_no_delay_value,
                                                      tcp_no_delay_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'TCP_NODELAY' "
                                              << "is OFF ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                                else {
                                    std::cout << "NOTE: 'TCP_NODELAY' "
                                              << "by default ("
                                              << "socket = " << new_sd << ")."
                                              << std::endl;
                                }
                            }
                            while(false);
#else // USE_RANDOM_TCP_NO_DELAY
                            std::cout << "NOTE: 'TCP_NODELAY' "
                                      << "by default ("
                                      << "socket = " << new_sd << ")."
                                      << std::endl;
#endif // USE_RANDOM_TCP_NO_DELAY

#if defined(USE_RANDOM_LINGER_RST) && USE_RANDOM_LINGER_RST == 1
                            do {
                                std::random_device rd;
                                std::mt19937 gen(rd());
                                std::uniform_int_distribution<> dis(0, 2);
                                struct linger linger_value;
                                socklen_t linger_value_size =
                                        sizeof(linger_value);
                                int res_gen = dis(gen);

    #if defined(FORCE_LINGER_RST)
        #if FORCE_LINGER_RST == 0
                            res_gen = 0;
        #elif FORCE_LINGER_RST == 1
                            res_gen = 1;
        #else // FORCE_LINGER_RST == 2
                            res_gen = 2;
        #endif // FORCE_LINGER_RST
    #endif // FORCE_LINGER_RST

                                if(0 == res_gen) {
                                    linger_value.l_onoff = 1;
                                    linger_value.l_linger = 0;
                                    rc = ::setsockopt(new_sd, SOL_SOCKET,
                                                      SO_LINGER, &linger_value,
                                                      linger_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'LINGER' is ON "
                                              << "(socket = "
                                              << new_sd << "). "
                                              << "Send RST on disconnect"
                                              << std::endl;
                                }
                                else if(1 == res_gen) {
                                    linger_value.l_onoff = 0;
                                    linger_value.l_linger = 0;
                                    rc = ::setsockopt(new_sd, SOL_SOCKET,
                                                      SO_LINGER, &linger_value,
                                                      linger_value_size);
                                    assert(rc >= 0);

                                    std::cout << "NOTE: 'LINGER' is OFF "
                                              << "(socket = "
                                              << new_sd << "). "
                                              << "Try send FIN on disconnect."
                                              << std::endl;
                                }
                                else {
                                    std::cout << "NOTE: 'LINGER' by default "
                                              << "(is not enable) "
                                              << "socket = " << new_sd << ")."
                                              << "Try send FIN (or RST) "
                                              << "on disconnect."
                                              << std::endl;
                                }
                            }
                            while(false);
#else // USE_RANDOM_LINGER_RST
                            std::cout << "NOTE: 'LINGER' by default "
                                      << "(is not enable) "
                                      << "socket = " << new_sd << ")."
                                      << "Try send FIN (or RST) "
                                      << "on disconnect."
                                      << std::endl;
#endif // USE_RANDOM_LINGER_RST

                            fds[nfds].fd = new_sd;
                            fds[nfds].events = POLLIN | POLLOUT;
                            nfds++;

                            fds[nfds].fd = fd;
                            fds[nfds].events = POLLIN;
                            nfds++;

                            counter_sent[new_sd] = 0;
                            counter_recv[new_sd] = 0;
                            start_time_work[new_sd] =
                                    std::chrono::system_clock::now();
                        }
                        else {
                            std::cout << "> "
                                      << "Can't add new connection!"
                                      << std::endl;
                        }
                    }
                    else {
                        if(i % 2) {
#if defined(DEBUG)
                            std::cout << ">>> SOCKET (i=" << i << ")"
                                      << std::endl;
#endif // DEBUG
                            if(fds[i].revents & POLLIN) {
                                rc = ::recv(fds[i].fd, buffer,
                                            sizeof(buffer), 0);
                                if(rc < 0) {
                                    if(errno != EWOULDBLOCK) {
                                        std::cout << "ERROR: recv "<< i <<" ("
                                                  << fds[i].fd << "): "
                                                  << strerror(errno)
                                                  << std::endl;
                                        print_nfds();
                                        assert(false);
                                    }
                                    else {
                                        std::cout << ">>> EWOULDBLOCK on recv"
                                                  << std::endl;
                                    }
                                }
                                else if(0 == rc) {
                                    std::cout << "> "
                                              << "Connection close (socket): "
                                              << fds[i].fd << ":"
                                              << fds[i + 1].fd
                                              << "."
                                              << get_str_info(
                                              fds[i].fd,
                                              std::chrono::system_clock::now())
                                              << std::endl;
                                    close_socket(i);
                                    break;
                                }
                                else {
                                    len = rc;

                                    counter_recv[fds[i].fd] += len;
                                }
                            }
                        }
                        else {
#if defined(DEBUG)
                            std::cout << ">>> FILE (i=" << i << ")"
                                      << std::endl;
#endif // DEBUG
                            if((fds[i].revents & POLLIN) &&
                               (fds[i - 1].revents & POLLOUT)) {
                                rc = ::read(fds[i].fd, buffer, sizeof(buffer));
                                assert(rc >= 0);

                                if(0 == rc) {
                                    std::cout << "> "
                                              << "Connection close (file): "
                                              << fds[i - 1].fd << ":"
                                              << fds[i].fd
                                              << "."
                                              << get_str_info(
                                              fds[i - 1].fd,
                                              std::chrono::system_clock::now())
                                              << std::endl;
                                    close_file(i);
                                    break;
                                }
                                else {
                                    len = rc;

                                    int index = 0;
                                    int count = 0;

                                    do {
                                        rc = ::send(fds[i - 1].fd,
                                                    &buffer[index], len, 0);
                                        if(rc < 0) {
                                            if(errno != EWOULDBLOCK) {
                                                std::cout << "ERROR: "
                                                          << "'sent' failed ("
                                                          << "socket="
                                                          << fds[i - 1].fd
                                                          << "): "
                                                          << ::strerror(errno)
                                                          << std::endl;
                                                break;
                                            }
                                            else {
                                                std::cout << ">>> EWOULDBLOCK "
                                                          << " on send: "
                                                          << count++
                                                          << "("
                                                          << global_count++
                                                          << ")." << std::endl;
                                                (void) ::sleep(1);
                                                continue;
                                            }
                                        }
                                        else {
                                            counter_sent[fds[i - 1].fd] += rc;
                                            if(rc != len) {
                                                len = len - rc;
                                                index += rc;
                                                continue;
                                            }
                                        }
                                    }
                                    while(false);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(compress_array) {
            compress_array = false;
            for(int i = 0; i < nfds;) {
                if(fds[i].fd == -1) {
                    for(int j = i; j < nfds; j++) {
                        fds[j].fd = fds[j + 1].fd;
                    }
                    nfds--;
                }
                else {
                    i++;
                }
            }
        }
    }
    while(true);

    std::cout << "End of program." << std::endl;

    return EXIT_SUCCESS;
}

/* *****************************************************************************
 * End of file
 * ************************************************************************** */
