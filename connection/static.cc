#include "connection.h"

unordered_map<string, string> Connection::_status_map = {
  {"200", "OK"},
  {"302", "Found"},
  {"400", "Bad Request"},
  {"404", "Not Found"},
  {"500", "Internal Server Error"},
};
