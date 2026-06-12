/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "http.hpp"

#include "ALog.h"
#include "event_loop.hpp"
#include "tcp_socket.hpp"
#include "thread.hpp"

#include <cerrno>
#include <cstring>
#include <map>
#include <utility>

namespace aplay {
namespace protocol {
namespace http {

static const char* kLogTag = "APlayHttp";

class HttpServer::Impl : public aplay::core::Thread {
public:
    Impl()
        : aplay::core::Thread("aplay-httpd"), loop_(), parser_(), listener_(),
          clients_(), handler_(), started_(false), port_(0) {}

    bool start_server(std::uint16_t port, const RequestHandler& handler) {
        if (started_ || !handler) {
            return false;
        }

        listener_ = aplay::core::socket::open_ipv4_tcp_socket();
        if (!listener_.valid()) {
            LOGE(kLogTag, "failed to open TCP socket");
            return false;
        }

        listener_.set_reuse_address(true);
        listener_.set_nonblocking(true);

        aplay::core::socket::Ipv4Endpoint endpoint;
        endpoint.address = 0;
        endpoint.port = port;
        if (!listener_.bind_to(endpoint)) {
            LOGE(kLogTag, "failed to bind TCP port %u: %s",
                 static_cast<unsigned>(port), std::strerror(errno));
            listener_.close();
            return false;
        }

        aplay::core::socket::Ipv4Endpoint bound_endpoint;
        if (!listener_.local_endpoint(&bound_endpoint)) {
            LOGE(kLogTag, "failed to query bound TCP port: %s", std::strerror(errno));
            listener_.close();
            return false;
        }
        port_ = bound_endpoint.port;

        if (!listener_.listen(16)) {
            LOGE(kLogTag, "failed to listen on TCP port %u: %s",
                 static_cast<unsigned>(port_), std::strerror(errno));
            listener_.close();
            port_ = 0;
            return false;
        }

        handler_ = handler;
        if (!loop_.add_reader(listener_.fd(), [this](int fd) { on_accept(fd); })) {
            listener_.close();
            port_ = 0;
            return false;
        }

        started_ = start();
        if (!started_) {
            loop_.remove(listener_.fd());
            listener_.close();
            port_ = 0;
            return false;
        }

        LOGI(kLogTag, "HTTPD listening on 0.0.0.0:%u", static_cast<unsigned>(port_));
        return true;
    }

    void stop_server() {
        if (!started_) {
            return;
        }

        loop_.stop();
        stopAndJoin();
        for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
            it->second.socket.close();
        }
        clients_.clear();
        if (listener_.valid()) {
            loop_.remove(listener_.fd());
            listener_.close();
        }
        started_ = false;
        port_ = 0;
        LOGI(kLogTag, "HTTPD stopped");
    }

    bool running_server() const {
        return started_;
    }

    std::uint16_t server_port() const {
        return port_;
    }

private:
    struct Client {
        aplay::core::socket::TcpSocket socket;
        std::string buffer;
        bool reverse_http = false;
    };

    bool runOnce() override {
        return loop_.run_once(1000);
    }

    void on_accept(int) {
        while (true) {
            aplay::core::socket::Ipv4Endpoint peer;
            aplay::core::socket::TcpSocket client = listener_.accept(&peer);
            if (!client.valid()) {
                return;
            }
            client.set_nonblocking(true);
            const int fd = client.fd();
            Client state;
            state.socket = std::move(client);
            clients_[fd] = std::move(state);
            loop_.add_reader(fd, [this](int readable_fd) { on_client(readable_fd); });
        }
    }

    void on_client(int fd) {
        std::map<int, Client>::iterator it = clients_.find(fd);
        if (it == clients_.end()) {
            return;
        }

        std::uint8_t bytes[4096];
        while (true) {
            const int received = it->second.socket.receive(bytes, sizeof(bytes));
            if (received > 0) {
                it->second.buffer.append(reinterpret_cast<const char*>(bytes),
                                         static_cast<std::size_t>(received));
                continue;
            }
            if (received == 0) {
                close_client(fd);
                return;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            close_client(fd);
            return;
        }

        if (it->second.reverse_http) {
            it->second.buffer.clear();
            return;
        }

        Request request;
        std::string error;
        const ParseStatus status = parser_.parse(it->second.buffer, request, error);
        if (status == ParseStatus::Incomplete) {
            return;
        }

        Response response =
            status == ParseStatus::Complete ? handler_(request)
                                            : make_text_response(400, "Bad Request", error + "\n");
        write_response(it->second.socket, response);
        if (status == ParseStatus::Complete && response.status_code == 101) {
            LOGI(kLogTag, "upgraded connection fd=%d to reverse HTTP", fd);
            it->second.reverse_http = true;
            it->second.buffer.clear();
            return;
        }
        close_client(fd);
    }

    void write_response(const aplay::core::socket::TcpSocket& socket,
                        const Response& response) const {
        const std::string bytes = response.serialize();
        std::size_t sent_total = 0;
        while (sent_total < bytes.size()) {
            const int sent =
                socket.send(reinterpret_cast<const std::uint8_t*>(bytes.data() + sent_total),
                            bytes.size() - sent_total);
            if (sent <= 0) {
                return;
            }
            sent_total += static_cast<std::size_t>(sent);
        }
    }

    void close_client(int fd) {
        loop_.remove(fd);
        clients_.erase(fd);
    }

    aplay::core::EventLoop loop_;
    RequestParser parser_;
    aplay::core::socket::TcpSocket listener_;
    std::map<int, Client> clients_;
    RequestHandler handler_;
    bool started_;
    std::uint16_t port_;
};

HttpServer::HttpServer() : impl_(new Impl()) {}

HttpServer::~HttpServer() {
    delete impl_;
}

bool HttpServer::start(std::uint16_t port, const RequestHandler& handler) {
    return impl_->start_server(port, handler);
}

void HttpServer::stop() {
    impl_->stop_server();
}

bool HttpServer::running() const {
    return impl_->running_server();
}

std::uint16_t HttpServer::port() const {
    return impl_->server_port();
}

} // namespace http
} // namespace protocol
} // namespace aplay
