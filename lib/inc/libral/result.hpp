#pragma once

#include <boost/optional.hpp>

namespace libral {
  /*
     A poor man's emulation of Rust's Result construct. The basic idea is
     that we want to avoid throwing exceptions and rather force our callers
     to be explicit about how they handle errors.

     One day we might be able to use std::expected but that's still
     experimental

     We would really like to use boost::variant here, but that runs into
     trouble along the lines of
     https://svn.boost.org/trac/boost/ticket/7120 so we do it the
     old-fashioned way with a lot more boilerplate code
  */

  // The basic error we use everywhere
  struct error {
    error(const std::string &det) : detail(det) {};
    std::string detail;
  };

  // Indication that something is not implemented
  struct not_implemented_error : public error {
    not_implemented_error() : error("not implemented") { }
  };


  /* A result is either an error or whatever we really wanted */
  template <class R>
  class result {
  private:
    enum class tag { err, ok };
  public:
    result(R& ok) : _tag(tag::ok), _ok(ok) {};
    result(const R& ok) : _tag(tag::ok), _ok(ok) {};
    result(R&& ok) : _tag(tag::ok), _ok(std::move(ok)) {};
    result(error& err) : _tag(tag::err), _err(err) {};
    result(const error& err) : _tag(tag::err), _err(err) {};
    result(error&& err) : _tag(tag::err), _err(std::move(err)) {};
    result(const result& other) {
      if (other.is_ok()) {
        _ok = other._ok;
      } else {
        if (_tag == tag::err) {
          _err = other._err;
        } else {
          new ((void*) &_err) error(other._err);
        }
      }
    }

    result& operator=(const result& other) {
      if (other.is_ok()) {
        _ok = other._ok;
      } else {
        if (_tag == tag::err) {
          _err = other._err;
        } else {
          new ((void*) &_err) error(other._err);
        }
      }
      return *this;
    }

    ~result() {
      if (is_ok()) {
        _ok.~R();
      } else {
        _err.~error();
      }
    }

    boost::optional<R&> ok() {
      if (is_ok()) {
        return this->_ok;
      } else {
        return boost::none;
      }
    };

    boost::optional<const R&> ok() const {
      if (is_ok()) {
        return this->_ok;
      } else {
        return boost::none;
      }
    };

    boost::optional<error&> err() {
      if (is_err()) {
        return this->_err;
      } else {
        return boost::none;
      }
    };

    boost::optional<const error&> err() const {
      if (is_err()) {
        return this->_err;
      } else {
        return boost::none;
      }
    };

    bool is_ok()  const { return _tag == tag::ok; }
    bool is_err() const { return _tag == tag::err; }

    static std::unique_ptr<result<R>> make_unique() {
      return std::unique_ptr<result<R>>(new result<R>(R()));
    };

    static std::unique_ptr<result<R>> make_unique(const error& e) {
      return std::unique_ptr<result<R>>(new result<R>(e));
    };
  private:
    tag _tag;
    union {
      error _err;
      R     _ok;
    };
  };
}
