#include "connection.h"

unordered_map<string, string> Connection::_status_map = {
    {"200", "OK"},
    {"400", "Bad Request"},
    {"404", "Not Found"},
};
