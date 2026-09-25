#include "Config.hpp"
#include "SSLContext.hpp"
#include <QFile>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/impl/socket_ops.ipp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <string>
#include "Task.hpp"
#include <thread>
#include <utility>
#include <vector>

#ifndef SESSION_
#define SESSION_
#endif