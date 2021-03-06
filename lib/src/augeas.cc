#include <libral/augeas.hpp>

#include <boost/nowide/iostream.hpp>

namespace aug {
  node::node(std::shared_ptr<handle> aug, const std::string path)
    : _aug(aug), _path(path) { }

  std::string node::append(const std::string& p2) const {
    if (p2.front() == '/') {
      return p2;
    }
    if (_path.back() == '/') {
      return _path + p2;
    } else {
      return _path + "/" + p2;
    }
  }

  boost::optional<std::string> node::operator[](const std::string& path) const {
    return _aug->value(append(path));
  }

  void node::set(const std::string& path, const std::string& value) {
    _aug->set(append(path), value);
  }

  void node::set(const std::string& path,
                 const boost::optional<std::string>& value,
                 const std::string& deflt) {
    if (value) {
      _aug->set(append(path), *value);
    } else {
      _aug->set(append(path), deflt);
    }
  }

  void node::set_maybe(const std::string& path,
                       const boost::optional<std::string>& value) {
    if (value) {
      _aug->set(append(path), *value);
    }
  }

  void node::erase() {
    _aug->rm(append("*"));
  }

  void node::rm() {
    _aug->rm(_path);
  }

  void node::clear() {
    _aug->clear(_path);
  }

  void handle::include(const std::string& lens, const std::string& glob) {
    aug_transform(_augeas, lens.c_str(), glob.c_str(), 0);
    check_error();
  }

  void handle::load(void) {
    aug_load(this->_augeas);
    // @todo lutter 2016-06-03: give callers a decent way to figure out if
    // all their files were read successfully; not convinced that
    // check_error() in this case is the right interface
  }

  void handle::save(void) {
    aug_save(this->_augeas);
    check_error();
    for (const auto &e : match("/augeas//error")) {
      std::cerr << "error:" << e.path() << std::endl;
    }
  }

  std::vector<node> handle::match(const std::string& pathx) {
    std::vector<node> result;
    char **matches;
    int r = aug_match(_augeas, pathx.c_str(), &matches);
    check_error();

    for (int i=0; i < r; i++) {
      result.push_back(make_node(std::string(matches[i])));
      free(matches[i]);
    }
    free(matches);
    return result;
  }

  // @todo lutter 2016-06-03: really ? No return type ?
  void handle::set(const std::string& path, const std::string& value) {
    aug_set(_augeas, path.c_str(), value.c_str());
    check_error();
  }

  void handle::clear(const std::string& path) {
    aug_set(_augeas, path.c_str(), NULL);
    check_error();
  }

  // @todo lutter 2016-06-03: really ? No return type ?
  void handle::rm(const std::string& path) {
    aug_rm(_augeas, path.c_str());
    check_error();
  }

  boost::optional<std::string> handle::value(const std::string& pathx) const {
    const char *value;

    aug_get(_augeas, pathx.c_str(), &value);
    check_error();
    if (value == NULL) {
      return boost::none;
    } else {
      return std::string(value);
    }
  }

  static std::string to_string(const char *s) {
    if (s == NULL) {
      return "";
    } else {
      return std::string(s);
    }
  }

  node handle::make_node(const std::string& path) {
    return aug::node(shared_from_this(), path);
  }

  void handle::check_error() const {
    if (aug_error(_augeas) != AUG_NOERROR) {
      // @todo lutter 2016-05-10: produce real error messages
      // these will be impossible ot comprehend
      auto msg = to_string(aug_error_message(_augeas)) +
        "\n" + to_string(aug_error_minor_message(_augeas)) +
        "\n" + to_string(aug_error_details(_augeas));
      throw error { msg };
    }
  }
}
