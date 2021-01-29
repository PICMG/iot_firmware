#include "Terminus.h"

Terminus::Terminus() : deviceHandle(0) {

}

Terminus::~Terminus() {
    if (deviceHandle) uart_close(deviceHandle);
}
