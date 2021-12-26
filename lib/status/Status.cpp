#include "Status.h"

Status& Status::instance() {
    static Status o;
    return o;
}
